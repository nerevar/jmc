/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: alias.c - funtions related the the alias command            */
/*                             TINTIN III                            */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                     coded by peter unold 1992                     */
/*********************************************************************/
#include "stdafx.h"
#include "tintin.h"

extern void show_aliases();
extern struct listnode *search_node_with_wild();
extern struct listnode *searchnode_list();

/**********************/
/* the #alias command */
/**********************/
BOOL show_aliases(char* left, CGROUP* pGroup)
{
    BOOL bFound = FALSE;

    if ( !left || !*left ) 
        left = "*";
    
    ALIAS_INDEX ind = AliasList.begin();
    while (ind  != AliasList.end() ) {
        if ( (!pGroup || (pGroup == ind->second->m_pGroup) ) && match(left, (char*)ind->first.data() ) ){
            ALIAS* pal = ind->second;
            char temp[BUFFER_SIZE];
            sprintf(temp, rs::rs(1009), (char*)ind->first.data(), (char*)(pal->m_strRight.data()), (char*)pal->m_pGroup->m_strName.data()  );
            tintin_puts2(temp);
            bFound = TRUE;
        }
        ind++;
    }
    return bFound;
}

void alias_command(char *arg)
{
    char left[BUFFER_SIZE], right[BUFFER_SIZE], arg2[BUFFER_SIZE], group[BUFFER_SIZE];

    arg=get_arg_in_braces(arg, left,  STOP_SPACES);
    arg=get_arg_in_braces(arg, right, WITH_SPACES);

    if(!*left) {
        tintin_puts2(rs::rs(1010));
        // strcpy(left, "*");
    }

    if(!*right) {
        if ( !show_aliases(left) ) 
            tintin_puts2(rs::rs(1011));
        return;
    }
    // now create alias
    arg=get_arg_in_braces(arg, group, STOP_SPACES);

    // check for ' ' in left side
    char* ptr = left;
    while ( *ptr && *ptr != ' ') 
        ptr++;
    *ptr = 0;

    ALIAS_INDEX ind = AliasList.find(left);
    ALIAS* pal;
    if ( ind == AliasList.end() ) {
        pal= new ALIAS;
    } else {
        pal = ind->second;
    }

    pal->SetGroup(group);

    pal->m_strRight = right;
    pal->m_strLeft = left;
    AliasList[left] = pal;

    if (mesvar[MSG_ALIAS]) {
        sprintf(arg2, rs::rs(1012), left, right, pal->m_pGroup->m_strName.data());
        tintin_puts2(arg2);
    }
}

/************************/
/* the #unalias command */
/************************/
void unalias_command(char *arg)
{
    char left[BUFFER_SIZE], result[BUFFER_SIZE];
    BOOL bFound = FALSE;
  
    arg=get_arg_in_braces(arg, left, WITH_SPACES);

    ALIAS_INDEX ind = AliasList.begin();

    while (ind  != AliasList.end() ) {
        if ( match(left, (char*)ind->first.data()) ){
            ALIAS* pal = ind->second;
            if (mesvar[MSG_ALIAS]) {
                sprintf(result, rs::rs(1013), (char*)ind->first.data());
                tintin_puts2(result);
            }
            bFound = TRUE;
            ALIAS_INDEX ind1 = ind++;
            AliasList.erase(ind1);
            delete pal;
        } else 
            ind++;
    }
  
    if (!bFound && mesvar[MSG_ALIAS]) {
        sprintf(result, rs::rs(1007), left);
        tintin_puts2(result);
    }    
}


void DLLEXPORT RemoveAlias(char* name) 
{
    ALIAS_INDEX ind = AliasList.find(name);
    if ( ind != AliasList.end() ) {
        delete ind->second;
        AliasList.erase(ind);
    }
}

PALIAS DLLEXPORT SetAlias(char* name, char* text, char* group) 
{
    ALIAS_INDEX ind = AliasList.find(name);
    ALIAS* pal;
    if ( ind == AliasList.end() ) {
        pal= new ALIAS;
        pal->m_strLeft = name;
    } else {
        pal = ind->second;
    }

    pal->SetGroup(group);

    pal->m_strRight = text;
    AliasList[name] = pal;
    return pal;
}

PPALIAS DLLEXPORT GetAliasList(int* size)
{
    *size = AliasList.size();

    ALIAS_INDEX ind = AliasList.begin();
    int i = 0;
    while (ind != AliasList.end() ) {
        JMCObjRet[i++] = ind->second;
        ind++;
    }
    return (PPALIAS)JMCObjRet;
}

PALIAS DLLEXPORT GetAlias(char* name)
{
    ALIAS_INDEX ind = AliasList.find(name);
    if ( ind == AliasList.end() ) 
        return NULL;
    else 
        return ind->second;

}

