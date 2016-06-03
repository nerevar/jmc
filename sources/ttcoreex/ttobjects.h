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
    CGROUP(const wchar_t* name, BOOL bGlobal = FALSE);
    ~CGROUP();
    std::wstring m_strName;
    BOOL m_bEnabled, m_bGlobal;
};
typedef CGROUP* PCGROUP;
typedef CGROUP** PPCGROUP;

class GROUPED_NODE {
public:
    GROUPED_NODE();
    CGROUP* m_pGroup;
    virtual void SetGroup(const wchar_t* group = NULL);
};

class CPCRE {
public:
	CPCRE();
	~CPCRE();

	std::wstring m_strSource;
	BOOL m_bContainVars, m_bMultiline, m_bIgnoreCase;

	pcre16 *m_pPcre;
	pcre16_extra *m_pExtra;

	void Clear(BOOL ResetSource);
	BOOL SetSource(const std::wstring Source, BOOL Multiline, BOOL IgnoreCase);

	BOOL Recompile(const wchar_t *Pattern = NULL);
};
typedef CPCRE* PCPCRE;

class ALIAS : public GROUPED_NODE {
public:
	ALIAS();
	~ALIAS();

    std::wstring m_strRight;
    std::wstring m_strLeft;
	
	CPCRE m_PCRE;

	BOOL m_bDeleted;
	BOOL m_bRecompile;

	BOOL SetLeft(const wchar_t *left);
	
};
typedef ALIAS* PALIAS;
typedef ALIAS** PPALIAS;

class ACTION : public GROUPED_NODE {
public: 
    ACTION();
    ~ACTION();
    int m_nPriority;

    std::wstring m_strLeft;
    std::wstring m_strRight;
    
	CPCRE m_PCRE;

	BOOL m_bGlobal;

    BOOL m_bDeleted;

	enum ActionType {
		Action_TEXT = 0,
		Action_RAW = 1,
		Action_COLOR = 2
	};
	ActionType m_InputType;

    bool operator <(const ACTION*& y) const {
        return false; 
    };

    BOOL m_bRecompile;

    BOOL SetLeft(const wchar_t* left);
};
typedef ACTION* PACTION;
typedef ACTION** PPACTION;


class VAR {
public :
    VAR(wchar_t* val = NULL, BOOL bGlobal = FALSE);
    std::wstring m_strVal;
    BOOL m_bGlobal;
};

class HLIGHT : public GROUPED_NODE
{
public:
    std::wstring m_strPattern;
    std::wstring m_strColor;
    std::wstring m_strAnsi;
    BOOL SetColor(const wchar_t* color);
};
typedef HLIGHT* PHLIGHT;
typedef HLIGHT** PPHLIGHT;

typedef struct {
    wchar_t* m_strName;
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
    std::wstring m_strAction;
    std::wstring m_strKey;
};
typedef CHotKey* PCHotKey;
typedef CHotKey** PPCHotKey;

//vls-begin// script files
class CScriptFile {
public:
    std::wstring m_strName;
};
typedef CScriptFile* PCScriptFile;
//vls-end//

// Group operations 
void DLLEXPORT RemoveGroup(const wchar_t* name) ;
PCGROUP DLLEXPORT SetGroup(const wchar_t* name, BOOL bEnabled, BOOL bGlobal) ;
PPCGROUP DLLEXPORT GetGroupsList(int* size); 
PCGROUP DLLEXPORT GetGroup(const wchar_t* name);


// alias operations 
void DLLEXPORT RemoveAlias(const wchar_t* name) ;
PALIAS DLLEXPORT SetAlias(const wchar_t* name, const wchar_t* text, const wchar_t* group) ;
PPALIAS DLLEXPORT GetAliasList(int* size); 
PALIAS DLLEXPORT GetAlias(const wchar_t* name);

// action operations 
//void DLLEXPORT RemoveAction(wchar_t* name) ;
BOOL DLLEXPORT RemoveAction(ACTION* pac);
PACTION DLLEXPORT SetAction(ACTION::ActionType type, const wchar_t* name, const wchar_t* text, int priority, const wchar_t* group) ;
void DLLEXPORT SetActionText(ACTION* pac, const wchar_t* text) ;
void DLLEXPORT SetActionPattern(PACTION pAct, const wchar_t* strText);
PPACTION DLLEXPORT GetActionsList(int* size); 
PACTION DLLEXPORT GetAction(const wchar_t* name);

// hlight operations 
void DLLEXPORT RemoveHlight(const wchar_t* pattern) ;
PHLIGHT DLLEXPORT SetHlight(const wchar_t* color, const wchar_t* pattern, const wchar_t* group) ;
PPHLIGHT DLLEXPORT GetHlightList(int* size); 
PHLIGHT DLLEXPORT GetHlight(const wchar_t* pattern);

//vls-begin// subst page
void DLLEXPORT RemoveSubst(const wchar_t* pattern);
LPWSTR DLLEXPORT SetSubst(const wchar_t* text, const wchar_t* pattern);
void DLLEXPORT SetSubstPattern(const wchar_t* pSubst, const const wchar_t* strPattern);
void DLLEXPORT GetSubstList(int* size); 
LPWSTR DLLEXPORT GetSubst(int pos);
LPWSTR DLLEXPORT GetSubstText(const wchar_t* pattern);
//vls-end//

// hotkey operations 
PPCHotKey DLLEXPORT GetHotList(int* size); 
PCHotKey DLLEXPORT GetHot(WORD wScanCode, WORD wAltState);
//vls-begin// grouped hotkeys
//PCHotKey DLLEXPORT SetHot(WORD wScanCode, WORD wAltState, wchar_t* strKeyString, wchar_t* action) ;
PCHotKey DLLEXPORT SetHot(WORD wScanCode, WORD wAltState, const wchar_t* strKeyString, const wchar_t* action, const wchar_t* group);
//vls-end//
void DLLEXPORT RemoveHot(CHotKey* pHot);
void DLLEXPORT SetHotText(CHotKey* hot, const wchar_t* text) ;

//vls-begin// script files
PCScriptFile DLLEXPORT GetScriptFileList(int* size);
PCScriptFile DLLEXPORT GetScriptFile(int ind);
int DLLEXPORT FindScriptFile(const wchar_t *filename);
PCScriptFile DLLEXPORT AddScriptFile(const wchar_t* filename);
void DLLEXPORT UpScriptFile(const wchar_t* filename);
void DLLEXPORT DownScriptFile(const wchar_t *filename);
void DLLEXPORT RemoveScriptFile(const wchar_t *filename);
//vls-end//

#endif // _TTOBJECTS_H_

