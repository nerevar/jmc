#include "stdafx.h"
#include "tintin.h"

void* JMCObjRet[1000];

void group_command(wchar_t* arg)
{
    wchar_t option[BUFFER_SIZE], name [BUFFER_SIZE], result [BUFFER_SIZE];
//vls-begin// bugfix
    wchar_t* ErrMsg = rs::rs(1047);
//vls-end//

    arg=get_arg_in_braces(arg,option,STOP_SPACES,sizeof(option)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,name,STOP_SPACES,sizeof(name)/sizeof(wchar_t)-1);

    if ( !*option ) {
        tintin_puts2(ErrMsg);
        return;
    }

    if ( is_abrev(option, L"list") ){
        GROUP_INDEX ind = GroupList.begin ();
        tintin_puts2(rs::rs(1048));
        while ( ind != GroupList.end() ) {
            wchar_t buf [BUFFER_SIZE];
            wcscpy (buf, ind->first.c_str());
            if ( ind->second->m_bEnabled == FALSE ) 
                wcscat (buf, rs::rs(1049));
            if ( ind->second->m_bGlobal == TRUE ) 
                wcscat (buf, rs::rs(1050));
            tintin_puts2(buf);
            ind++;
        }
        return;
    } 

    GROUP_INDEX ind = GroupList.find(name);
    if ( ind == GroupList .end() ) {
        swprintf ( result, rs::rs(1051), name);
        tintin_puts2(result);
        return;
    }
    
    if (is_abrev(option, L"disable") ){
        ind->second->m_bEnabled = FALSE;
        if ( mesvar[MSG_GRP] ) {
            swprintf ( result, rs::rs(1052), name);
            tintin_puts2(result);
        }
        return;
    } else 
    if ( is_abrev(option, L"enable") ){
        ind->second->m_bEnabled = TRUE;
        if ( mesvar[MSG_GRP] ) {
            swprintf ( result, rs::rs(1053), name);
            tintin_puts2(result);
        }
        return;
    } else 
    if ( is_abrev(option, L"delete") ){
        delete ind->second;
        GroupList.erase(ind);
        if ( mesvar[MSG_GRP] ) {
            swprintf ( result, rs::rs(1054), name);
            tintin_puts2(result);
        }
        return;
    } else 
    if ( is_abrev(option, L"global") ){
        ind->second->m_bGlobal = TRUE;
        swprintf ( result, rs::rs(1055), name);
        tintin_puts2(result);
        return;
    } else 
    if ( is_abrev(option, L"local") ){
        ind->second->m_bGlobal = FALSE;
        swprintf ( result, rs::rs(1056), name);
        tintin_puts2(result);
        return;
    } else 
    if ( is_abrev(option, L"info") ){
        swprintf ( result, rs::rs(1057), name);
        if ( ind->second->m_bGlobal )
            wcscat(result, rs::rs(1058));
        if ( !ind->second->m_bEnabled )
            wcscat(result, rs::rs(1059));
        tintin_puts2(result);
        
        tintin_puts2(rs::rs(1060));
        show_aliases(NULL, ind->second);

        tintin_puts2(rs::rs(1061));
        show_actions(NULL, ind->second);

        tintin_puts2(rs::rs(1062));
        show_high(ind->second);

        return;
    } else {

        tintin_puts2(ErrMsg);
    }
}

PPCGROUP DLLEXPORT GetGroupsList(int* size)
{
    *size = GroupList.size();

    GROUP_INDEX ind = GroupList.begin();
    int i = 0;
    while (ind != GroupList.end() ) {
        JMCObjRet[i++] = ind->second;
        ind++;
    }
    return (PPCGROUP)JMCObjRet;
}

PCGROUP DLLEXPORT GetGroup(const wchar_t* name)
{
    GROUP_INDEX ind = GroupList.find(name);
    if ( ind == GroupList.end() ) 
        return NULL;
    else 
        return ind->second;

}

void DLLEXPORT RemoveGroup(const wchar_t* name) 
{
    GROUP_INDEX ind = GroupList.find(name);
    if ( ind == GroupList.end() ) 
        return ;

    delete ind->second;
    GroupList.erase(ind);
}

PCGROUP DLLEXPORT SetGroup(const wchar_t* name, BOOL bEnabled, BOOL bGlobal) 
{
    GROUP_INDEX ind = GroupList.find(name);
    CGROUP* pGrp;
    if ( ind == GroupList.end() ) {
        pGrp = new CGROUP(name, bGlobal);
        pGrp->m_bEnabled = bEnabled;
        GroupList[name] = pGrp;
    } else {
        pGrp = ind->second;
        pGrp->m_bEnabled = bEnabled;
        pGrp->m_bGlobal = bGlobal;
    }
    return pGrp;
}
