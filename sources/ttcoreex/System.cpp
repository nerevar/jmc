//vls-begin// #system
// #system, #ps & #term.
//
// List of launched processes stored in STL

#include "stdafx.h"
#include "ttcoreex.h"
#include "tintin.h"
#include <algorithm>


extern DIRECT_OUT_FUNC DirectOutputFunction;

class PS_ITEM
{
public:
    HANDLE m_hRead;         // process output stream
    DWORD m_dwPID;          // Process ID
    std::string m_strCmd;   // command line

public:
    PS_ITEM(): m_hRead(NULL), m_dwPID(0)
    {}
    PS_ITEM(const PS_ITEM* psi): m_hRead(psi->m_hRead), m_dwPID(psi->m_dwPID), m_strCmd(psi->m_strCmd)
    {}
    PS_ITEM(const PS_ITEM& psi): m_hRead(psi.m_hRead), m_dwPID(psi.m_dwPID), m_strCmd(psi.m_strCmd)
    {}
    PS_ITEM(HANDLE hRead, DWORD dwPID, const char* sCmd): m_hRead(hRead), m_dwPID(dwPID), m_strCmd(sCmd)
    {}
    ~PS_ITEM() {}

    PS_ITEM& operator= (const PS_ITEM& psi)
    {
        m_hRead = psi.m_hRead;
        m_dwPID = psi.m_dwPID;
        m_strCmd = psi.m_strCmd;
    }
};

typedef std::list<PS_ITEM*> PS_LIST;
typedef PS_LIST::iterator PS_INDEX;

PS_LIST PsList;

void systemlist_delete(DWORD dwPID)
{
    PS_ITEM* psi;
    EnterCriticalSection(&secSystemList);
    PS_INDEX ind = PsList.begin();
    while (ind != PsList.end())
    {
        psi = *ind;
        if (psi->m_dwPID == dwPID)
        {
            delete psi;
            PsList.erase(ind);
            break;
        }
        ind++;
    }
    LeaveCriticalSection(&secSystemList);
}

unsigned long __stdcall systemexec_thread(void * pParam)
{
    PS_ITEM* psi = (PS_ITEM*)pParam;
    char buf[4096];
    char msg[4096 + 1]; // +1 for '\0'
    DWORD nr = 1;

    while (ReadFile(psi->m_hRead, buf, sizeof(buf), &nr, NULL) && nr > 0)
    {
        OemToCharBuff(buf, msg, nr);
        msg[nr] = '\0';

        EnterCriticalSection(&secSystemExec);
		tintin_puts2(msg);
        LeaveCriticalSection(&secSystemExec);
    }
    CloseHandle(psi->m_hRead);
    systemlist_delete(psi->m_dwPID);
    delete psi;
    return 0;
}

void systemexec_command(char *arg)
{
    char cmd[BUFFER_SIZE];
//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    get_arg_in_braces(arg, cmd, STOP_SPACES);
    get_arg_in_braces(arg, cmd, WITH_SPACES);
//* /en
    if ( !*cmd ) {
        tintin_puts2(rs::rs(1224));
        return;
    }

    HANDLE hOutRead = NULL, hOutDup = NULL;
    HANDLE hOutWrite = NULL, hErrWrite = NULL;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hOutRead, &hOutWrite, &sa, BUFFER_SIZE)) {
systemexec_error:
        CloseHandle(hErrWrite);
        CloseHandle(hOutWrite);
        CloseHandle(hOutRead);
        tintin_puts2(rs::rs(1195));
        return;
    }

    if (!DuplicateHandle(
        GetCurrentProcess(), hOutWrite,
        GetCurrentProcess(), &hErrWrite , 0,
        TRUE, DUPLICATE_SAME_ACCESS))
        goto systemexec_error;

    if (!DuplicateHandle(
        GetCurrentProcess(), hOutRead,
        GetCurrentProcess(), &hOutDup , 0,
        FALSE, DUPLICATE_SAME_ACCESS))
        goto systemexec_error;

    CloseHandle(hOutRead);

    PROCESS_INFORMATION pi;
    STARTUPINFO         si;
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    //si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdError = hErrWrite;
    si.hStdOutput = hOutWrite;

    if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE,
        CREATE_NO_WINDOW | DETACHED_PROCESS, NULL, NULL, &si, &pi))
        goto systemexec_error;

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(hErrWrite);
    CloseHandle(hOutWrite);

    PS_ITEM* psi = new PS_ITEM(hOutDup, pi.dwProcessId, cmd);
    PS_ITEM* pParam = new PS_ITEM(psi);

    EnterCriticalSection(&secSystemList);
    PsList.push_front(psi);
    LeaveCriticalSection(&secSystemList);

    DWORD dwId;
    HANDLE hThread = CreateThread(NULL, 0,
        (LPTHREAD_START_ROUTINE)&systemexec_thread, (LPVOID)pParam, 0, &dwId);
    if (!hThread) {
        delete psi;
        systemlist_delete(pi.dwProcessId);
        return;
    }

    CloseHandle(hThread);
}

void systemlist_command(char *arg)
{
    tintin_puts2(rs::rs(1226));

    EnterCriticalSection(&secSystemList);
    PS_INDEX ind = PsList.begin();
    while (ind != PsList.end()) {
        PS_ITEM* psi = *ind;
        HANDLE hp = OpenProcess(PROCESS_TERMINATE, FALSE, psi->m_dwPID);
        if (hp) {
            char str[BUFFER_SIZE];
            _snprintf(str, BUFFER_SIZE, rs::rs(1227), psi->m_dwPID, psi->m_strCmd.data());
            tintin_puts2(str);
            CloseHandle(hp);
            ind++;
        } else {
            delete psi;
            ind = PsList.erase(ind);
        }
    }
    LeaveCriticalSection(&secSystemList);
}

void systemkill_command(char *arg)
{
    DWORD pid = 0;
    char word[BUFFER_SIZE];
//* en
//* due to an error in tintin.h (messed up STOP_SPACES and WITH_SPACES)
//    get_arg_in_braces(arg, word, WITH_SPACES);
    get_arg_in_braces(arg, word, STOP_SPACES);
//* /en
    if ( !*word ) {
systemkill_error:
        tintin_puts2(rs::rs(1225));
        return;
    }

    if (!strcmp(word, "all")) {
        // pid = 0;
    } else if (!strcmp(word, "last")) {
        // pid = PsList.size() > 0 ? (*(PsList.begin()))->m_dwPID : 0;
        if (PsList.size() > 0) pid = (*(PsList.begin()))->m_dwPID;
    } else {
        pid = atoi(word);
        if (pid == 0) goto systemkill_error;
    }

    BOOL bFound = FALSE;

    EnterCriticalSection(&secSystemList);
    PS_INDEX ind = PsList.begin();
    while (ind != PsList.end()) {
        PS_ITEM* psi = *ind;
        if (pid == 0 || psi->m_dwPID == pid) {
            bFound = TRUE;
            HANDLE hp = OpenProcess(PROCESS_TERMINATE, FALSE, psi->m_dwPID);
            if (hp) TerminateProcess(hp, 0);
            delete psi;
            ind = PsList.erase(ind);
        } else {
            ind++;
        }
    }
    LeaveCriticalSection(&secSystemList);
    if (!bFound && pid != 0) {
        tintin_puts2(rs::rs(1228));
    }
}
//vls-end//
