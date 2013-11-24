#include "stdafx.h"
#include "tintin.h"
#include "tintinx.h"
#include <windows.h>
#include <commctrl.h>

std::map <WORD, CHotKey*> HotkeyList;
CRITICAL_SECTION DLLEXPORT secHotkeys;

KEYNAME KeyNames[] ={
    {"INS", 82 , 1},
    {"HOME", 71 , 1},
    {"PGUP", 73 ,1},
    {"DEL", 83 ,1},
    {"END", 79 ,1},
    {"PGDN", 81 ,1},
    {"UP", 72 ,1},
    {"LEFT",  75 ,1},
    {"DOWN", 80 ,1}, 
    {"RIGHT", 77 , 1},
    {"DIV", 53 ,1},
    {"MUL", 55 , 0},
    {"MIN", 74 ,0},
    {"NUM7", 71 ,0},
    {"NUM8", 72 ,0},
    {"NUM9", 73 ,0},
    {"NUM4",75 ,0},
    {"NUM5", 76 ,0},
    {"NUM6",77 ,0},
    {"NUM1",79 ,0},
    {"NUM2", 80 ,0},
    {"NUM3", 81 ,0},
    {"NUM0",82 ,0},
    {"NUMDEL", 83 ,0},
    {"ADD", 78 ,0},
    {"RETURN", 28 , 1},
    {"BACK", 14 ,0},
    {"TAB", 15 , 0},
    {"ENTER", 28 , 0},
    {"ESC",1 ,0},
    {"SP",57 ,0},
    {":", 39 ,0},
    {"=",13 , 0},
    {"," ,51 , 0},
    {"-", 12 ,0},
    {".", 52 ,0},
    {"/",53 , 0},
    {"`", 41 , 0},
    {"[",26 ,0},
    {"\\", 43 ,0},
    {"]", 27 ,0},
    {"'", 40 , 0}
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


static BOOL GetHotKeyParams(char* strKey , WORD& ScanCode, WORD& AltState, char* strKeyCode)
{
    int i;
    AltState = 0;
    ScanCode = 0;;
    _tcsupr(strKey);

    if ( strstr(strKey, "ALT+" )  )
        AltState += HOTKEYF_ALT;
    if ( strstr(strKey, "CTRL+") ) 
        AltState += HOTKEYF_CONTROL;
    if ( strstr(strKey, "SHIFT+") ) 
        AltState += HOTKEYF_SHIFT;


    char* ptr = strrchr(strKey, '+' );
    if ( !ptr ) 
        strcpy(strKeyCode, strKey);
    else 
        strcpy(strKeyCode, ptr+1);

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
        if ( !strcmp(KeyNames[i].m_strName , strKeyCode ) ) {
            ScanCode = KeyNames[i].m_scancode ;
            if ( KeyNames[i].m_Ext ) 
                AltState += HOTKEYF_EXT;
            return TRUE;
        }
    }
    // check for F* keys
    if ( *strKeyCode == 'F' && *(strKeyCode+1) ) {
        int n = atoi(strKeyCode+1);
        if ( n == 0 ) 
            return FALSE;
        ScanCode = MapVirtualKey (VK_F1 + n -1, 0);
        return TRUE;
    }

    // only printable characters left 
    if ( strKeyCode[1] ) 
        return FALSE;

	if((*strKeyCode >= '0' && *strKeyCode <= '9') ||
        (*strKeyCode >= 'A' && *strKeyCode <= 'Z')) {
        ScanCode = MapVirtualKey (*strKeyCode, 0);
        return TRUE;
    }

    return FALSE;
}


void SetHotKey(char* arg)
{
    char left[BUFFER_SIZE], right[BUFFER_SIZE];
//vls-begin// grouped hotkeys
    char group[BUFFER_SIZE];
//vls-end//

    arg=get_arg_in_braces(arg, left,  STOP_SPACES);
    arg=get_arg_in_braces(arg, right, WITH_SPACES);
//vls-begin// grouped hotkeys
    arg=get_arg_in_braces(arg, group, STOP_SPACES);
//vls-end//

    if(!*left ) {
        // show al hotkeys defined 
        tintin_puts2(rs::rs(1076));
        EnterCriticalSection(&secHotkeys);
        HOTKEY_INDEX ind = HotkeyList.begin();
        while ( ind != HotkeyList.end() ) {
            CHotKey* phot = ind->second;
            char buff[BUFFER_SIZE];
//vls-begin// grouped hotkeys
//            sprintf ( buff, rs::rs(1077) , phot->m_strKey.data() , phot->m_strAction.data() );
            sprintf ( buff, rs::rs(1077) , phot->m_strKey.data() , phot->m_strAction.data(),
                phot->m_pGroup->m_strName.data());
//vls-end//
            tintin_puts2(buff);
            ind++;
        }
        LeaveCriticalSection(&secHotkeys);
        return;
    }

    WORD AltState , ScanCode ;
    char strKeyCode[32];
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
            pKey->m_strKey += "Ctrl+";
        if ( pKey->m_nAltState & HOTKEYF_ALT ) 
            pKey->m_strKey += "Alt+";
        if ( pKey->m_nAltState & HOTKEYF_SHIFT ) 
            pKey->m_strKey += "Shift+";
        pKey->m_strKey+=strKeyCode ;
        HotkeyList[key] = pKey;
    }
    else {
        pKey = ind->second;
        if ( !*right ) {
            char buff[BUFFER_SIZE];
//vls-begin// grouped hotkeys
//            sprintf ( buff, rs::rs(1077) , pKey->m_strKey.data() , pKey->m_strAction.data() );
            sprintf ( buff, rs::rs(1077) , pKey->m_strKey.data() , pKey->m_strAction.data(),
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
        char msg[BUFFER_SIZE];
//vls-begin// grouped hotkeys
//        sprintf(msg, rs::rs(1080), pKey->m_strKey.data() , pKey->m_strAction.data());
        sprintf(msg, rs::rs(1080), pKey->m_strKey.data() , pKey->m_strAction.data(),
            pKey->m_pGroup->m_strName.data());
//vls-end//
        tintin_puts2(msg);
    }
}

LPCSTR DLLEXPORT GetHotkeyValue(WORD key)
{
    static char ret[BUFFER_SIZE];
    EnterCriticalSection(&secHotkeys);
    HOTKEY_INDEX ind = HotkeyList.find (key);
    if ( ind != HotkeyList.end() ) {
        CHotKey* pKey = ind->second;
//vls-begin// grouped hotkeys
//        strcpy((char*)ret ,  pKey->m_strAction.data ());
//        LeaveCriticalSection(&secHotkeys);
//        return ret;
        if (pKey->m_pGroup->m_bEnabled) {
            strcpy((char*)ret ,  pKey->m_strAction.data ());
            LeaveCriticalSection(&secHotkeys);
            return ret;
        }
//vls-end//
    }
    LeaveCriticalSection(&secHotkeys);
    return NULL;
}

void Unhotkey(char* arg)
{
    char strkey[BUFFER_SIZE];
    get_arg_in_braces(arg, strkey, STOP_SPACES);
    if ( !*strkey) {
//        if ( !bSilent ) 
            tintin_puts2(rs::rs(1081));
        return;
    }

    WORD AltState, ScanCode ;;
    char strKeyCode[32];
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
            char buff[BUFFER_SIZE];
            sprintf(buff, rs::rs(1083) , pKey->m_strKey.data() );
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
//PCHotKey DLLEXPORT SetHot(WORD wKeyCode, WORD wAltState, char* strKeyString, char* action) 
PCHotKey DLLEXPORT SetHot(WORD wKeyCode, WORD wAltState, char* strKeyString, char* action, char* group) 
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

void DLLEXPORT SetHotText(CHotKey* hot, char* text)
{

    hot->m_strAction = text;

}