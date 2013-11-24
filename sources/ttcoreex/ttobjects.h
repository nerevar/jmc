#ifndef _TTOBJECTS_H_
#define _TTOBJECTS_H_

#include <string>
#include <map>
#include <list>
#include "../recore/pcre.h"

class TIMER 
{
public:
    TIMER(int ID, int Interval, int PreInterval) ;
    int m_nID, m_nInterval, m_nPreinterval;
    DWORD m_dwLastTickCount;
    BOOL m_bPreTimerDone;
};


class CGROUP {
public:
    CGROUP();
    CGROUP(char* name, BOOL bGlobal = FALSE);
    ~CGROUP();
    std::string m_strName;
    BOOL m_bEnabled, m_bGlobal;
};
typedef CGROUP* PCGROUP;
typedef CGROUP** PPCGROUP;

class GROUPED_NODE {
public:
    GROUPED_NODE();
    CGROUP* m_pGroup;
    virtual void SetGroup(char* group = NULL);
};


class ALIAS : public GROUPED_NODE {
public:
    std::string m_strRight;
    std::string m_strLeft;
    
};
typedef ALIAS* PALIAS;
typedef ALIAS** PPALIAS;

class ACTION : public GROUPED_NODE {
public: 
    ACTION();
    ~ACTION();
    int m_nPriority;
    std::string m_strLeft;
    std::string m_strRight;
    std::string m_strRegex;
    pcre* m_pPcre;
    pcre_extra*  m_pExtra;

    BOOL m_bDeleted;

    bool operator <(const ACTION*& y) const {
        return false; 
    };

    BOOL m_bRecompile;

    BOOL SetLeft(char* left);
    BOOL CreatePattern(char* left = NULL);
};
typedef ACTION* PACTION;
typedef ACTION** PPACTION;


class VAR {
public :
    VAR(char* val = NULL, BOOL bGlobal = FALSE);
    std::string m_strVal;
    BOOL m_bGlobal;
};

class HLIGHT : public GROUPED_NODE
{
public:
    std::string m_strPattern;
    std::string m_strColor;
    std::string m_strAnsi;
    BOOL SetColor(char* color);
};
typedef HLIGHT* PHLIGHT;
typedef HLIGHT** PPHLIGHT;

typedef struct {
    char* m_strName;
    BYTE m_scancode;
    BYTE m_Ext;
} KEYNAME;
typedef KEYNAME* PKEYNAME;

extern PKEYNAME DLLEXPORT HotKeyNames;
extern int DLLEXPORT HotKeyNamesSize;

//vls-begin// grouped hotkeys
//class CHotKey {
class CHotKey : public GROUPED_NODE {
//vls-end//
public:
    CHotKey();
    CHotKey(short Code , BOOL Alt , BOOL Ctrl, BOOL Shift);
    WORD m_nAltState, m_nScanCode;
    std::string m_strAction;
    std::string m_strKey;
};
typedef CHotKey* PCHotKey;
typedef CHotKey** PPCHotKey;

//vls-begin// script files
class CScriptFile {
public:
    std::string m_strName;
};
typedef CScriptFile* PCScriptFile;
//vls-end//

// Group operations 
void DLLEXPORT RemoveGroup(char* name) ;
PCGROUP DLLEXPORT SetGroup(char* name, BOOL bEnabled, BOOL bGlobal) ;
PPCGROUP DLLEXPORT GetGroupsList(int* size); 
PCGROUP DLLEXPORT GetGroup(char* name);


// alias operations 
void DLLEXPORT RemoveAlias(char* name) ;
PALIAS DLLEXPORT SetAlias(char* name, char* text, char* group) ;
PPALIAS DLLEXPORT GetAliasList(int* size); 
PALIAS DLLEXPORT GetAlias(char* name);

// action operations 
//void DLLEXPORT RemoveAction(char* name) ;
BOOL DLLEXPORT RemoveAction(ACTION* pac);
PACTION DLLEXPORT SetAction(char* name, char* text, int priority, char* group) ;
void DLLEXPORT SetActionText(ACTION* pac, char* text) ;
PPACTION DLLEXPORT GetActionsList(int* size); 
PACTION DLLEXPORT GetAction(char* name);
void DLLEXPORT SetActionPattern(PACTION pAct, LPCSTR strText);

// hlight operations 
void DLLEXPORT RemoveHlight(char* pattern) ;
PHLIGHT DLLEXPORT SetHlight(char* color, char* pattern, char* group) ;
PPHLIGHT DLLEXPORT GetHlightList(int* size); 
PHLIGHT DLLEXPORT GetHlight(char* pattern);

//vls-begin// subst page
void DLLEXPORT RemoveSubst(char* pattern);
LPSTR DLLEXPORT SetSubst(char* text, char* pattern);
void DLLEXPORT SetSubstPattern(LPCSTR pSubst, LPCSTR strPattern);
void DLLEXPORT GetSubstList(int* size); 
LPSTR DLLEXPORT GetSubst(int pos);
LPSTR DLLEXPORT GetSubstText(char* pattern);
//vls-end//

// hotkey operations 
PPCHotKey DLLEXPORT GetHotList(int* size); 
PCHotKey DLLEXPORT GetHot(WORD wScanCode, WORD wAltState);
//vls-begin// grouped hotkeys
//PCHotKey DLLEXPORT SetHot(WORD wScanCode, WORD wAltState, char* strKeyString, char* action) ;
PCHotKey DLLEXPORT SetHot(WORD wScanCode, WORD wAltState, char* strKeyString, char* action, char* group);
//vls-end//
void DLLEXPORT RemoveHot(CHotKey* pHot);
void DLLEXPORT SetHotText(CHotKey* hot, char* text) ;

//vls-begin// script files
PCScriptFile DLLEXPORT GetScriptFileList(int* size);
PCScriptFile DLLEXPORT GetScriptFile(int ind);
int DLLEXPORT FindScriptFile(char *filename);
PCScriptFile DLLEXPORT AddScriptFile(char* filename);
void DLLEXPORT UpScriptFile(char* filename);
void DLLEXPORT DownScriptFile(char *filename);
void DLLEXPORT RemoveScriptFile(char *filename);
//vls-end//

#endif // _TTOBJECTS_H_

