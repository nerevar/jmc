#include "stdafx.h"
#include "tintin.h"
#include "tintinx.h"
#include <windows.h>
#include <commctrl.h>

std::map <WORD, CHotKey*> HotkeyList;
CRITICAL_SECTION DLLEXPORT secHotkeys;

KEYNAME KeyNames[] ={
    {L"INS", 82 , 1},
    {L"HOME", 71 , 1},
    {L"PGUP", 73 ,1},
    {L"DEL", 83 ,1},
    {L"END", 79 ,1},
    {L"PGDN", 81 ,1},
    {L"UP", 72 ,1},
    {L"LEFT",  75 ,1},
    {L"DOWN", 80 ,1}, 
    {L"RIGHT", 77 , 1},
    {L"DIV", 53 ,1},
    {L"MUL", 55 , 0},
    {L"MIN", 74 ,0},
    {L"NUM7", 71 ,0},
    {L"NUM8", 72 ,0},
    {L"NUM9", 73 ,0},
    {L"NUM4",75 ,0},
    {L"NUM5", 76 ,0},
    {L"NUM6",77 ,0},
    {L"NUM1",79 ,0},
    {L"NUM2", 80 ,0},
    {L"NUM3", 81 ,0},
    {L"NUM0",82 ,0},
    {L"NUMDEL", 83 ,0},
    {L"ADD", 78 ,0},
    {L"RETURN", 28 , 1},
    {L"BACK", 14 ,0},
    {L"TAB", 15 , 0},
    {L"ENTER", 28 , 0},
    {L"ESC",1 ,0},
    {L"SP",57 ,0},
    {L":", 39 ,0},
    {L"=",13 , 0},
    {L"," ,51 , 0},
    {L"-", 12 ,0},
    {L".", 52 ,0},
    {L"/",53 , 0},
    {L"`", 41 , 0},
    {L"[",26 ,0},
    {L"\\", 43 ,0},
    {L"]", 27 ,0},
    {L"'", 40 , 0}
};

PKEYNAME DLLEXPORT HotKeyNames = KeyNames;
int DLLEXPORT HotKeyNamesSize = sizeof(KeyNames)/sizeof(KEYNAME);

CHotKey::CHotKey()
{
    m_nAltState =0;
    m_nScanCode = 0;
}

CHotKey::CHotKey(short Code , BOOL Alt , BOOL Ctrl, BOOL Shift)
{
    m_nScanCode = Code;
    m_nAltState = 0;
    if ( Alt )
        m_nAltState += HOTKEYF_ALT;
    if ( Ctrl )
        m_nAltState += HOTKEYF_CONTROL;
    if (Shift )
        m_nAltState += HOTKEYF_SHIFT;
}


static BOOL GetHotKeyParams(wchar_t* strKey , WORD& ScanCode, WORD& AltState, wchar_t* strKeyCode)
{
    int i;
    AltState = 0;
    ScanCode = 0;;
    _wcsupr(strKey);

    if ( wcsstr(strKey, L"ALT+" )  )
        AltState += HOTKEYF_ALT;
    if ( wcsstr(strKey, L"CTRL+") ) 
        AltState += HOTKEYF_CONTROL;
    if ( wcsstr(strKey, L"SHIFT+") ) 
        AltState += HOTKEYF_SHIFT;


    wchar_t* ptr = wcschr(strKey, L'+' );
    if ( !ptr ) 
        wcscpy(strKeyCode, strKey);
    else 
        wcscpy(strKeyCode, ptr+1);

    if ( *strKeyCode == 0  ) {
        tintin_puts2(rs::rs(1075));
        return FALSE;
    }

    // start .finding scancodes
    int ext;
    if ( AltState&HOTKEYF_EXT ) {
        ext = 1;
    }
    else 
        ext = 0;

    // first check for present in the Names table
    for ( i = 0 ; i < HotKeyNamesSize  ; i++ ) {
        if ( !wcscmp(KeyNames[i].m_strName , strKeyCode ) ) {
            ScanCode = KeyNames[i].m_scancode ;
            if ( KeyNames[i].m_Ext ) 
                AltState += HOTKEYF_EXT;
            return TRUE;
        }
    }
    // check for F* keys
    if ( *strKeyCode == L'F' && *(strKeyCode+1) ) {
        int n = _wtoi(strKeyCode+1);
        if ( n == 0 ) 
            return FALSE;
        ScanCode = MapVirtualKey (VK_F1 + n -1, 0);
        return TRUE;
    }

    // only printable characters left 
    if ( strKeyCode[1] ) 
        return FALSE;

	if (iswdigit(*strKeyCode) || iswupper(*strKeyCode)) {
        ScanCode = MapVirtualKey (*strKeyCode, 0);
        return TRUE;
    }

    return FALSE;
}


void SetHotKey(wchar_t* arg)
{
    wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE];
//vls-begin// grouped hotkeys
    wchar_t group[BUFFER_SIZE];
//vls-end//

    arg=get_arg_in_braces(arg,left,STOP_SPACES,sizeof(left)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,right,WITH_SPACES,sizeof(right)/sizeof(wchar_t)-1);
//vls-begin// grouped hotkeys
    arg=get_arg_in_braces(arg,group,STOP_SPACES,sizeof(group)/sizeof(wchar_t)-1);
//vls-end//

    if(!*left ) {
        // show al hotkeys defined 
        tintin_puts2(rs::rs(1076));
        EnterCriticalSection(&secHotkeys);
        HOTKEY_INDEX ind = HotkeyList.begin();
        while ( ind != HotkeyList.end() ) {
            CHotKey* phot = ind->second;
            wchar_t buff[BUFFER_SIZE];
            swprintf ( buff, rs::rs(1077) , phot->m_strKey.c_str() , phot->m_strAction.c_str(),
                phot->m_pGroup->m_strName.c_str());
            tintin_puts2(buff);
            ind++;
        }
        LeaveCriticalSection(&secHotkeys);
        return;
    }

    WORD AltState , ScanCode ;
    wchar_t strKeyCode[32];
    if ( !GetHotKeyParams (left, ScanCode, AltState, strKeyCode) ) 
        return;
    
    // ok , start find key
    EnterCriticalSection(&secHotkeys);

    WORD key = MAKEWORD(ScanCode, AltState);
    HOTKEY_INDEX ind = HotkeyList.find (key);
    CHotKey* pKey;

    if ( ind == HotkeyList.end() ) {
        if ( !*right ) {
            tintin_puts2 (rs::rs(1078));
            LeaveCriticalSection(&secHotkeys);
            return;
        }
        pKey = new CHotKey;
//vls-begin// grouped hotkeys
        pKey->SetGroup(group);
//vls-end//
        pKey->m_nAltState = AltState;
        pKey->m_strAction = right;
        pKey->m_nScanCode = ScanCode;

        

        if ( pKey->m_nAltState & HOTKEYF_CONTROL ) 
            pKey->m_strKey += L"Ctrl+";
        if ( pKey->m_nAltState & HOTKEYF_ALT ) 
            pKey->m_strKey += L"Alt+";
        if ( pKey->m_nAltState & HOTKEYF_SHIFT ) 
            pKey->m_strKey += L"Shift+";
        pKey->m_strKey+=strKeyCode ;
        HotkeyList[key] = pKey;
    }
    else {
        pKey = ind->second;
        if ( !*right ) {
            wchar_t buff[BUFFER_SIZE];
//vls-begin// grouped hotkeys
//            swprintf ( buff, rs::rs(1077) , pKey->m_strKey.data() , pKey->m_strAction.data() );
            swprintf ( buff, rs::rs(1077) , pKey->m_strKey.data() , pKey->m_strAction.data(),
                pKey->m_pGroup->m_strName.data());
//vls-end//
            LeaveCriticalSection(&secHotkeys);
            tintin_puts2(buff);
            return;
        } else 
            pKey->m_strAction = right;
    }
    LeaveCriticalSection(&secHotkeys);

    //tintin_puts2
    if ( mesvar[MSG_HOT] ) {
        wchar_t msg[BUFFER_SIZE];
//vls-begin// grouped hotkeys
//        swprintf(msg, rs::rs(1080), pKey->m_strKey.data() , pKey->m_strAction.data());
        swprintf(msg, rs::rs(1080), pKey->m_strKey.data() , pKey->m_strAction.data(),
            pKey->m_pGroup->m_strName.data());
//vls-end//
        tintin_puts2(msg);
    }
}

LPWSTR DLLEXPORT GetHotkeyValue(WORD key)
{
    static wchar_t ret[BUFFER_SIZE];
    EnterCriticalSection(&secHotkeys);
    HOTKEY_INDEX ind = HotkeyList.find (key);
    if ( ind != HotkeyList.end() ) {
        CHotKey* pKey = ind->second;
//vls-begin// grouped hotkeys
//        wcscpy((char*)ret ,  pKey->m_strAction.data ());
//        LeaveCriticalSection(&secHotkeys);
//        return ret;
        if (pKey->m_pGroup->m_bEnabled) {
            wcscpy(ret ,  pKey->m_strAction.c_str());
            LeaveCriticalSection(&secHotkeys);
            return ret;
        }
//vls-end//
    }
    LeaveCriticalSection(&secHotkeys);
    return NULL;
}

void Unhotkey(wchar_t* arg)
{
    wchar_t strkey[BUFFER_SIZE];
    get_arg_in_braces(arg,strkey,STOP_SPACES,sizeof(strkey)/sizeof(wchar_t)-1);
    if ( !*strkey) {
//        if ( !bSilent ) 
            tintin_puts2(rs::rs(1081));
        return;
    }

    WORD AltState, ScanCode ;;
    wchar_t strKeyCode[32];
    if ( !GetHotKeyParams (strkey, ScanCode, AltState, strKeyCode) ) 
        return;
    
    EnterCriticalSection(&secHotkeys);

    
    DWORD key = MAKEWORD(ScanCode , AltState);
    HOTKEY_INDEX ind = HotkeyList.find (key);
    CHotKey* pKey;

    if ( ind == HotkeyList.end() ) {
        LeaveCriticalSection(&secHotkeys);
//        if ( !bSilent ) 
            tintin_puts2(rs::rs(1082));
    } else {
        pKey = ind->second;
        HotkeyList.erase(ind);
        LeaveCriticalSection(&secHotkeys);
        if ( /*!bSilent &&*/ mesvar[MSG_HOT]) {
            wchar_t buff[BUFFER_SIZE];
            swprintf(buff, rs::rs(1083) , pKey->m_strKey.data() );
            tintin_puts2 (buff);
        }
        delete pKey;
    }
    
}


void DLLEXPORT RemoveHot(CHotKey* pHot) 
{
    DWORD key = MAKEWORD(pHot->m_nScanCode , pHot->m_nAltState );
//vls-begin// bugfix
    HOTKEY_INDEX ind = HotkeyList.find(key);
    if ( ind != HotkeyList.end() ) {
        delete ind->second;
        HotkeyList.erase(ind);
    }
//vls-end//
}



PPCHotKey DLLEXPORT GetHotList(int* size)
{
    *size = HotkeyList.size();

    HOTKEY_INDEX ind = HotkeyList.begin();
    int i = 0;
    while (ind != HotkeyList.end() ) {
        JMCObjRet[i++] = ind->second;
        ind++;
    }
    return (PPCHotKey)JMCObjRet;
}


PCHotKey DLLEXPORT GetHot(WORD wKeyCode, WORD wAltState)
{
    
    DWORD key = MAKEWORD(wKeyCode, wAltState);
    HOTKEY_INDEX ind = HotkeyList.find (key);
    if ( ind == HotkeyList.end() ) 
        return NULL;
    else 
        return ind->second;

}

//vls-begin// grouped hotkeys
//PCHotKey DLLEXPORT SetHot(WORD wKeyCode, WORD wAltState, wchar_t* strKeyString, wchar_t* action) 
PCHotKey DLLEXPORT SetHot(WORD wKeyCode, WORD wAltState, const wchar_t* strKeyString, const wchar_t* action, const wchar_t* group) 
//vls-end//
{
   
    DWORD key = MAKEWORD(wKeyCode, wAltState);
    HOTKEY_INDEX ind = HotkeyList.find (key);
    CHotKey* pKey;

    if ( ind == HotkeyList.end() ) {
        pKey = new CHotKey;
//vls-begin// grouped hotkeys
        pKey->SetGroup(group);
//vls-end//
        pKey->m_nAltState = wAltState;
        pKey->m_strAction = action;
        pKey->m_nScanCode = wKeyCode;
        pKey->m_strKey = strKeyString;

        HotkeyList[key] = pKey;
    }
    else {
        pKey = ind->second;
//vls-begin// grouped hotkeys
        pKey->SetGroup(group);
//vls-end//
        pKey->m_strAction = action;
    }
    return pKey;
}

void DLLEXPORT SetHotText(CHotKey* hot, const wchar_t* text)
{

    hot->m_strAction = text;

}