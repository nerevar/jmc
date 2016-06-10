/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: variables.c - functions related the the variables           */
/*                             TINTIN ++                             */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                     coded by peter unold 1992                     */
/*********************************************************************/
#include "stdafx.h"
#include "tintin.h"
#include "variables.h"

extern wchar_t *get_arg_in_braces();
extern struct listnode *search_node_with_wild();
extern struct listnode *searchnode_list();

/*************************/
/* the #variable command */
/*************************/
void var_command(wchar_t *arg)
{
    wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE], temp[BUFFER_SIZE], gname[BUFFER_SIZE];

    arg=get_arg_in_braces(arg,left,STOP_SPACES,sizeof(left)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,right,WITH_SPACES,sizeof(right)/sizeof(wchar_t)-1);
    if(!*left) {
        tintin_puts2(rs::rs(1212));
        wcscpy(left, L"*");
    }

    if(*left && !*right) {
        VAR_INDEX ind = VarList.begin();
        while (ind  != VarList.end() ) {
            if ( match(left, ind->first.c_str() ) ){
                VAR* pvar = ind->second;
				int maxlen = sizeof(temp)/sizeof(wchar_t) - ( pvar->m_bGlobal ? wcslen(rs::rs(1050)) : 0 ) - 1;
				int len = _snwprintf(temp, maxlen, rs::rs(1213), ind->first.c_str(), pvar->m_strVal.c_str() );
				if( len < 0 )
					len = maxlen;
				temp[len] = L'\0';
                if ( pvar->m_bGlobal ) 
                    wcscat(temp, rs::rs(1050));
                tintin_puts2(temp);
            }
            ind++;
        }
        return;
    }

    // check group 
    get_arg_in_braces(arg,gname,STOP_SPACES,sizeof(gname)/sizeof(wchar_t)-1);
    VAR_INDEX ind = VarList.find(left);
    VAR* pvar;

    wchar_t strVal[BUFFER_SIZE - MAX_VARNAME_LENGTH - 32];
    substitute_myvars(right, strVal, sizeof(strVal)/sizeof(wchar_t));

	bool changed = false;
    if ( ind != VarList.end() ) {
        pvar = ind->second;
		if (pvar->m_strVal != strVal) {
			changed = true;
			pvar->m_strVal = strVal;
		}
    }
    else {
		changed = true;
        pvar = new VAR(strVal);
        VarList[left] = pvar;
    }

	BOOL glob;
    if ( !wcscmp(gname, L"global" ) ) 
        glob = TRUE;
    else 
        glob = FALSE;
	
	if (pvar->m_bGlobal != glob) {
		pvar->m_bGlobal = glob;
		changed = true;
	}

	if (changed) {
		VARTOPCRE::iterator pcres = VarPcreDeps.find(left);
		if (pcres != VarPcreDeps.end())
			for (std::set<CPCRE*>::iterator ind = pcres->second.begin(); ind != pcres->second.end(); ind++) {
				CPCRE* p = *ind;
				p->Recompile();
			}
	}

    if (changed && mesvar[MSG_VAR]) {
		int maxlen = sizeof(temp)/sizeof(wchar_t) - wcslen(pvar->m_bGlobal ? rs::rs(1216) : rs::rs(1217)) - 1;
		int len = _snwprintf(temp, maxlen, rs::rs(1215), left, strVal);
		if( len < 0 )
			len = maxlen;
		temp[len] = L'\0';
		//swprintf(temp, rs::rs(1215),left, strVal, gname);
        wcscat(temp, pvar->m_bGlobal ? rs::rs(1216) : rs::rs(1217));
        
        tintin_puts2(temp);
    }
}
/************************/
/* the #unvar   command */
/************************/
void unvar_command(wchar_t *arg)
{
    wchar_t left[BUFFER_SIZE] ,result[BUFFER_SIZE];
    BOOL bFound = FALSE;

    arg=get_arg_in_braces(arg,left,WITH_SPACES,sizeof(left)/sizeof(wchar_t)-1);
    if ( !*left ) {
        tintin_puts2(rs::rs(1218));
        return;
    }

    VAR_INDEX ind = VarList.begin();
    while (ind  != VarList.end() ) {
        if ( match(left, ind->first.c_str() ) ){
            VAR* pvar = ind->second;
            if (mesvar[MSG_VAR]) {
                swprintf(result, rs::rs(1219), left);
                tintin_puts2(result);
            }
            bFound = TRUE;
            VAR_INDEX ind1 = ind++;
            VarList.erase (ind1);
            delete pvar;
        } else 
            ind++;
    }
  
    if (!bFound && mesvar[MSG_VAR])
        tintin_puts2(rs::rs(1220));
}
/*************************************************************************/
/* copy the arg text into the result-space, but substitute the variables */
/* $<string> with the values they stand for                              */
/*************************************************************************/
void substitute_myvars(const wchar_t *arg, wchar_t *result, int maxlength)
{
	wchar_t varname[MAX_VARNAME_LENGTH+1];
	int nest=0,counter,varlen;

	maxlength--; //reserve for null-terminator
	while(*arg && maxlength > 0) {

		if(*arg==L'$') { /* substitute variable */
			counter=0;
			while (*(arg+counter)==L'$')
				counter++;
			varlen=0;
						
			while (is_allowed_symbol(*(arg+varlen+counter)))
				varlen++;
			
			if (varlen > MAX_VARNAME_LENGTH)
				varlen = MAX_VARNAME_LENGTH;
			if (varlen>0)
				wcsncpy(varname,arg+counter,varlen);
			*(varname+varlen)=L'\0';
			
			if (counter==nest+1 && !iswdigit(*(arg+counter))) {
				// check for date/time variable here !
				
				wchar_t specialVariableValue[BUFFER_SIZE];
				specialVariableValue[0] = L'\0';
				
				// find special variables
				for(int i=0;i<JMC_SPECIAL_VARIABLES_NUM;i++)
					if (!wcscmp(varname, jmc_vars[i].name)) {
						(*(jmc_vars[i].jmcfn))(specialVariableValue);
						break;
					}				

				if (wcslen(specialVariableValue) > 0) {
					// add value of special variable to "result"

					int len = wcslen(specialVariableValue);
					if (len > maxlength)
						len = maxlength;
					wcsncpy(result, specialVariableValue, len);
					result += len;
					maxlength -= len;
				
					arg+=counter+varlen;
				} else if (get_oob_variable(varname, specialVariableValue, sizeof(specialVariableValue) / sizeof(wchar_t) - 1) > 0) {
					int len = wcslen(specialVariableValue);
					if (len > maxlength)
						len = maxlength;
					wcsncpy(result, specialVariableValue, len);
					result += len;
					maxlength -= len;
				
					arg+=counter+varlen;
				} else {
					VAR_INDEX ind = VarList.find(varname);
					if( ind != VarList.end() ) {
						VAR* pvar = ind->second;

						int len = wcslen(pvar->m_strVal.c_str());
						if (len > maxlength)
							len = maxlength;
						wcsncpy(result, pvar->m_strVal.c_str(), len);
						result += len;
						maxlength -= len;

						arg+=counter+varlen;
					} else {
						int len = counter+varlen;
						if (len > maxlength)
							len = maxlength;
						wcsncpy(result, arg, len);
						result += len;
						maxlength -= len;

						arg+=varlen+counter;
					}
				}
			} else {
				int len = counter+varlen;
				if (len > maxlength)
					len = maxlength;
				wcsncpy(result, arg, len);
				result += len;
				maxlength -= len;

				arg+=varlen+counter;
			}
		} else if (*arg==DEFAULT_OPEN) {
			nest++;
			if (maxlength > 0) {
				*result++= *arg++;
				--maxlength;
			} else
				arg++;
		} else if (*arg==DEFAULT_CLOSE) {
			nest--;
			if (maxlength > 0) {
				*result++= *arg++;
				--maxlength;
			} else
				arg++;
		} else if (*arg==L'\\' && *(arg+1)==L'$' && nest==0) {
			arg++;
			if (maxlength > 0) {
				*result++= *arg++;
				--maxlength;
			} else
				arg++;
		} else if (maxlength > 0) {
		  *result++= *arg++;
		  --maxlength;
		} else
		  arg++;
	}
	
	*result=L'\0';
}

/*************************/
/* the #tolower command */
/*************************/
void tolower_command(wchar_t *arg)
{
    wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE], arg2[BUFFER_SIZE];

    arg = get_arg_in_braces(arg,left,STOP_SPACES,sizeof(left)/sizeof(wchar_t)-1);
    arg = get_arg_in_braces(arg,right,WITH_SPACES,sizeof(right)/sizeof(wchar_t)-1);
    if (!*left || !*right) {
        tintin_puts2(rs::rs(1221));
    } else {
        CharLower(right);
        VAR_INDEX vind = VarList.find(left);
        VAR* pvar;
        if ( vind != VarList.end() ) {
            vind->second->m_strVal = right;
        } else {
            pvar = new VAR(right);
            VarList[left] = pvar;
        }
        if (mesvar[MSG_VAR]) {
          swprintf(arg2, rs::rs(1222),left, right);
          tintin_puts2(arg2);
        }
    }
}

/*************************/
/* the #toupper command */
/*************************/
void toupper_command(wchar_t *arg)
{
    wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE], arg2[BUFFER_SIZE];

    arg = get_arg_in_braces(arg,left,STOP_SPACES,sizeof(left)/sizeof(wchar_t)-1);
    arg = get_arg_in_braces(arg,right,WITH_SPACES,sizeof(right)/sizeof(wchar_t)-1);
    if (!*left || !*right) {
        tintin_puts2(rs::rs(1223));
    } else {
        CharUpper(right);
        VAR_INDEX vind = VarList.find(left);
        VAR* pvar;
        if ( vind != VarList.end() ) {
            vind->second->m_strVal = right;
        } else {
            pvar = new VAR(right);
            VarList[left] = pvar;
        }
        if (mesvar[MSG_VAR]) {
          swprintf(arg2, rs::rs(1222),left, right);
          tintin_puts2(arg2);
        }
    }
}

/********************************************/
/* check symbol in variable name is allowed */
/********************************************/
bool is_allowed_symbol(wchar_t arg)
{
	return iswalpha(arg) || iswdigit(arg); 
}


// current user input
void variable_value_input(wchar_t *arg)
{
	wcscpy(arg, editStr);
}

// DATE AND TIME:
void variable_value_date(wchar_t *arg)
{
	SYSTEMTIME st;
	GetLocalTime(&st); 
    
	swprintf(arg, L"%02d-%02d-%d", st.wDay, st.wMonth, st.wYear);
}

void variable_value_year(wchar_t *arg)
{
	SYSTEMTIME st;
	GetLocalTime(&st); 
    
	swprintf(arg, L"%d", st.wYear);
}

void variable_value_month(wchar_t *arg)
{
	SYSTEMTIME st;
	GetLocalTime(&st); 
    
	swprintf(arg, L"%02d", st.wMonth);
}

void variable_value_day(wchar_t *arg)
{
	SYSTEMTIME st;
	GetLocalTime(&st); 
 
	swprintf(arg, L"%02d", st.wDay);
}

void variable_value_time(wchar_t *arg)
{
	SYSTEMTIME st;
	GetLocalTime(&st); 
    
	swprintf(arg, L"%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
}

void variable_value_hour(wchar_t *arg)
{
	SYSTEMTIME st;
	GetLocalTime(&st); 

    swprintf(arg, L"%02d", st.wHour);
}

void variable_value_minute(wchar_t *arg)
{
	SYSTEMTIME st;
	GetLocalTime(&st); 
    
	swprintf(arg, L"%02d", st.wMinute);
}

void variable_value_second(wchar_t *arg)
{
	SYSTEMTIME st;
	GetLocalTime(&st); 
    
	swprintf(arg, L"%02d", st.wSecond);
}

void variable_value_millisecond(wchar_t *arg)
{
	SYSTEMTIME st;
	GetLocalTime(&st); 
 
	swprintf(arg, L"%03d", st.wMilliseconds);
}

void variable_value_timestamp(wchar_t *arg)
{
	SYSTEMTIME st;
	FILETIME ft;
	ULARGE_INTEGER ularge;

	GetSystemTime(&st);
	SystemTimeToFileTime(&st,&ft);

	ularge.LowPart = ft.dwLowDateTime;
	ularge.HighPart = ft.dwHighDateTime;

	// convert filetime to unix timestamp
	swprintf(arg, L"%I64d", ularge.QuadPart / 10000000 - 11644473600);
}

void variable_value_clock(wchar_t *arg)
{
	swprintf(arg, L"%u", GetTickCount() / 100);
}

void variable_value_clockms(wchar_t *arg)
{
	swprintf(arg, L"%u", GetTickCount());
}

void variable_value_color_default(wchar_t *arg)
{
	swprintf(arg, L"%ls", DEFAULT_END_COLOR);
}

void variable_value_random(wchar_t *arg)
{
	//number of bits generated by single rand() should be calculated using RAND_MAX value
	unsigned long tmp = ((unsigned long)(rand() & 0xFF) << 24) |
		                ((unsigned long)(rand() & 0xFF) << 16) |
					    ((unsigned long)(rand() & 0xFF) <<  8) |
					    ((unsigned long)(rand() & 0xFF) <<  0) ;
	swprintf(arg, L"%u", tmp);
}

extern wchar_t MUDHostName[256];
void variable_value_hostname(wchar_t *arg)
{
	if (!MUDAddress.sin_addr.s_addr) {
		*arg = L'\0';
		return;
	}
	wcscpy(arg, MUDHostName);
}

void variable_value_hostip(wchar_t *arg)
{
	if (!MUDAddress.sin_addr.s_addr) {
		*arg = L'\0';
		return;
	}
	swprintf(arg, L"%d.%d.%d.%d",
		(MUDAddress.sin_addr.s_addr >>  0) & 0xff,
		(MUDAddress.sin_addr.s_addr >>  8) & 0xff,
		(MUDAddress.sin_addr.s_addr >> 16) & 0xff,
		(MUDAddress.sin_addr.s_addr >> 24) & 0xff);
}

void variable_value_hostport(wchar_t *arg)
{
	if (!MUDAddress.sin_addr.s_addr) {
		*arg = L'\0';
		return;
	}
	swprintf(arg, L"%d", htons(MUDAddress.sin_port));
}

void variable_value_eop(wchar_t *arg)
{
	swprintf(arg, L"%lc", END_OF_PROMPT_MARK);
}

void variable_value_eol(wchar_t *arg)
{
	swprintf(arg, L"%lc", L'\n');
}

void variable_value_esc(wchar_t *arg)
{
	swprintf(arg, L"%lc", ESC_SEQUENCE_MARK);
}

void variable_value_ping(wchar_t *arg)
{
	swprintf(arg, L"%d", lPingMUD);
}

void variable_value_ping_proxy(wchar_t *arg)
{
	swprintf(arg, L"%d", lPingProxy);
}

void variable_value_product_name(wchar_t *arg)
{
	swprintf(arg, L"%ls", strProductName);
}

void variable_value_product_version(wchar_t *arg)
{
	swprintf(arg, L"%ls", strProductVersion);
}
