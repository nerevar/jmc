#include "stdafx.h"
#include "tintin.h"

void* JMCObjRet[1000];

void group_command(char* arg)
{
    char option[BUFFER_SIZE], name [BUFFER_SIZE], result [BUFFER_SIZE];
//vls-begin// bugfix
    char* ErrMsg = rs::rs(1047);
//vls-end//

    arg=get_arg_in_braces(arg, option, STOP_SPACES);
    arg=get_arg_in_braces(arg, name,   STOP_SPACES);

    if ( !*option ) {
        tintin_puts2(ErrMsg);
        return;
    }

    if ( *option == 'l' && !strcmp(option, "list") ){
        GROUP_INDEX ind = GroupList.begin ();
        tintin_puts2(rs::rs(1048));
        while ( ind != GroupList.end() ) {
            char buf [BUFFER_SIZE];
            strcpy (buf, (char*)ind->first.data());
            if ( ind->second->m_bEnabled == FALSE ) 
                strcat (buf, rs::rs(1049));
            if ( ind->second->m_bGlobal == TRUE ) 
                strcat (buf, rs::rs(1050));
            tintin_puts2(buf);
            ind++;
        }
        return;
    } 

    GROUP_INDEX ind = GroupList.find(name);
    if ( ind == GroupList .end() ) {
        sprintf ( result, rs::rs(1051), name);
        tintin_puts2(result);
        return;
    }
    
    if ( *option == 'd' && !strcmp(option, "disable") ){
        ind->second->m_bEnabled = FALSE;
        if ( mesvar[MSG_GRP] ) {
            sprintf ( result, rs::rs(1052), name);
            tintin_puts2(result);
        }
        return;
    } else 
    if ( *option == 'e' && !strcmp(option, "enable") ){
        ind->second->m_bEnabled = TRUE;
        if ( mesvar[MSG_GRP] ) {
            sprintf ( result, rs::rs(1053), name);
            tintin_puts2(result);
        }
        return;
    } else 
    if ( *option == 'd' && !strcmp(option, "delete") ){
        delete ind->second;
        GroupList.erase(ind);
        if ( mesvar[MSG_GRP] ) {
            sprintf ( result, rs::rs(1054), name);
            tintin_puts2(result);
        }
        return;
    } else 
    if ( *option == 'g' && !strcmp(option, "global") ){
        ind->second->m_bGlobal = TRUE;
        sprintf ( result, rs::rs(1055), name);
        tintin_puts2(result);
        return;
    } else 
    if ( *option == 'l' && !strcmp(option, "local") ){
        ind->second->m_bGlobal = FALSE;
        sprintf ( result, rs::rs(1056), name);
        tintin_puts2(result);
        return;
    } else 
    if ( *option == 'i' && !strcmp(option, "info") ){
        sprintf ( result, rs::rs(1057), name);
        if ( ind->second->m_bGlobal )
            strcat(result, rs::rs(1058));
        if ( !ind->second->m_bEnabled )
            strcat(result, rs::rs(1059));
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

PCGROUP DLLEXPORT GetGroup(char* name)
{
    GROUP_INDEX ind = GroupList.find(name);
    if ( ind == GroupList.end() ) 
        return NULL;
    else 
        return ind->second;

}

void DLLEXPORT RemoveGroup(char* name) 
{
    GROUP_INDEX ind = GroupList.find(name);
    if ( ind == GroupList.end() ) 
        return ;

    delete ind->second;
    GroupList.erase(ind);
}

PCGROUP DLLEXPORT SetGroup(char* name, BOOL bEnabled, BOOL bGlobal) 
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
