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
BOOL show_aliases(wchar_t* left, CGROUP* pGroup)
{
    BOOL bFound = FALSE;

    if ( !left || !*left ) 
        left = L"*";
    
    ALIAS_INDEX ind = AliasList.begin();
    while (ind  != AliasList.end() ) {
        if ( (!pGroup || (pGroup == ind->second->m_pGroup) ) && match(left, ind->first.c_str() ) ){
            ALIAS* pal = ind->second;
            wchar_t temp[BUFFER_SIZE];
            swprintf(temp, rs::rs(1009), ind->first.c_str(), pal->m_strRight.c_str(), pal->m_pGroup->m_strName.c_str()  );
            tintin_puts2(temp);
            bFound = TRUE;
        }
        ind++;
    }
    return bFound;
}

void alias_command(wchar_t *arg)
{
    wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE], arg2[BUFFER_SIZE], group[BUFFER_SIZE];

    arg=get_arg_in_braces(arg,left,STOP_SPACES,sizeof(left)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,right,WITH_SPACES,sizeof(right)/sizeof(wchar_t)-1);

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
    arg=get_arg_in_braces(arg,group,STOP_SPACES,sizeof(group)/sizeof(wchar_t)-1);

    // check for ' ' in left side
	if (*left != L'/') {
		wchar_t* ptr = left;
		while ( *ptr && *ptr != L' ') 
			ptr++;
		*ptr = 0;
	}

    ALIAS_INDEX ind = AliasList.find(left);
    ALIAS* pal;
	BOOL bNew = FALSE;
    if ( ind == AliasList.end() ) {
        pal= new ALIAS();
		bNew = TRUE;
    } else {
        pal = ind->second;
    }

	if ( !pal->SetLeft (left) ) {
        if ( bNew ) {
            delete pal;
        }
        return;
    }
    pal->m_strRight = right;
    pal->SetGroup(group);

    AliasList[left] = pal;

    if (mesvar[MSG_ALIAS]) {
        swprintf(arg2, rs::rs(1012), left, right, pal->m_pGroup->m_strName.c_str());
        tintin_puts2(arg2);
    }
}

/************************/
/* the #unalias command */
/************************/
void unalias_command(wchar_t *arg)
{
    wchar_t left[BUFFER_SIZE], result[BUFFER_SIZE];
    BOOL bFound = FALSE;
  
    arg=get_arg_in_braces(arg,left,WITH_SPACES,sizeof(left)/sizeof(wchar_t)-1);

    ALIAS_INDEX ind = AliasList.begin();

    while (ind  != AliasList.end() ) {
		ALIAS* pal = ind->second;
		if ( (pal->m_PCRE.m_pPcre && !wcscmp(left, pal->m_strLeft.c_str())) ||
             (match(left, ind->first.c_str())) ) {
            if (mesvar[MSG_ALIAS]) {
                swprintf(result, rs::rs(1013), ind->first.c_str());
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
        swprintf(result, rs::rs(1007), left);
        tintin_puts2(result);
    }    
}


void DLLEXPORT RemoveAlias(const wchar_t* name) 
{
    ALIAS_INDEX ind = AliasList.find(name);
    if ( ind != AliasList.end() ) {
        delete ind->second;
        AliasList.erase(ind);
    }
}

PALIAS DLLEXPORT SetAlias(const wchar_t* name, const wchar_t* text, const wchar_t* group) 
{
    ALIAS_INDEX ind = AliasList.find(name);
    ALIAS* pal;
    if ( ind == AliasList.end() ) {
        pal= new ALIAS();
		if ( !pal->SetLeft (name) ) {
			pal->m_strLeft = name;
		}
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

PALIAS DLLEXPORT GetAlias(const wchar_t* name)
{
    ALIAS_INDEX ind = AliasList.find(name);
    if ( ind == AliasList.end() ) 
        return NULL;
    else 
        return ind->second;

}

