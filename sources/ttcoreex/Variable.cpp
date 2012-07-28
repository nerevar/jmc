/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: variables.c - functions related the the variables           */
/*                             TINTIN ++                             */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                     coded by peter unold 1992                     */
/*********************************************************************/
#include "stdafx.h"
#include "tintin.h"

extern char *get_arg_in_braces();
extern struct listnode *search_node_with_wild();
extern struct listnode *searchnode_list();

/*************************/
/* the #variable command */
/*************************/
void var_command(char *arg)
{
    char left[BUFFER_SIZE], right[BUFFER_SIZE], temp[BUFFER_SIZE], gname[BUFFER_SIZE];

    arg=get_arg_in_braces(arg, left,  STOP_SPACES);
    arg=get_arg_in_braces(arg, right, WITH_SPACES);
    if(!*left) {
        tintin_puts2(rs::rs(1212));
        strcpy(left, "*");
    }

    if(*left && !*right) {
        VAR_INDEX ind = VarList.begin();
        while (ind  != VarList.end() ) {
            if ( match(left, (char*)ind->first.data() ) ){
                VAR* pvar = ind->second;
                sprintf(temp, rs::rs(1213), (char*)ind->first.data(), (char*)pvar->m_strVal.data() );
                if ( pvar->m_bGlobal ) 
                    strcat(temp, rs::rs(1050));
                tintin_puts2(temp);
            }
            ind++;
        }
        return;
    }

    // check group 
    get_arg_in_braces(arg, gname, STOP_SPACES);
    VAR_INDEX ind = VarList.find(left);
    VAR* pvar;

    char strVal[BUFFER_SIZE];
    substitute_myvars(right, strVal);

    if ( ind != VarList.end() ) {
        pvar = ind->second;
        pvar->m_strVal = strVal;
    }
    else {
        pvar = new VAR(strVal);
        VarList[left] = pvar;
    }
    if ( !strcmp(gname, "global" ) ) 
        pvar->m_bGlobal = TRUE;
    else 
        pvar->m_bGlobal = FALSE;


    if (mesvar[MSG_VAR]) {
        sprintf(temp, rs::rs(1215),left, strVal, gname);
        if ( pvar->m_bGlobal  ) 
            strcat (temp , rs::rs(1216));
        else 
            strcat (temp , rs::rs(1217));
        tintin_puts2(temp);
    }
}
/************************/
/* the #unvar   command */
/************************/
void unvar_command(char *arg)
{
    char left[BUFFER_SIZE] ,result[BUFFER_SIZE];
    BOOL bFound = FALSE;

    arg=get_arg_in_braces(arg,left, WITH_SPACES);
    if ( !*left ) {
        tintin_puts2(rs::rs(1218));
        return;
    }

    VAR_INDEX ind = VarList.begin();
    while (ind  != VarList.end() ) {
        if ( match(left, (char*)ind->first.data() ) ){
            VAR* pvar = ind->second;
            if (mesvar[MSG_VAR]) {
                sprintf(result, rs::rs(1219), left);
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
void substitute_myvars(char *arg, char *result)
{
  char varname[20];
  int nest=0,counter,varlen;

  while(*arg) {

    if(*arg=='$') { /* substitute variable */
        counter=0;
        while (*(arg+counter)=='$')
	       counter++;
        varlen=0;
      // !!!! ATTENTION I AM NOT SURE ITS RIGHT CHANGE while(isalpha(*(arg+varlen+counter))) 
      while(isalpha(*(arg+varlen+counter)) || isdigit(*(arg+varlen+counter))) 
        varlen++;
      if (varlen>0)
        strncpy(varname,arg+counter,varlen);
      *(varname+varlen)='\0';
      if (counter==nest+1 && !isdigit(*(arg+counter+1))) {
        // check for date/time variable here !
//vls-begin// vars
          DWORD dw = 0;
          if ( varname[0] == 'D' && !strcmp(varname, "DATE"  ) ) dw = 1;
          if ( varname[0] == 'Y' && !strcmp(varname, "YEAR"  ) ) dw = 2;
          if ( varname[0] == 'M' && !strcmp(varname, "MONTH" ) ) dw = 3;
          if ( varname[0] == 'D' && !strcmp(varname, "DAY"   ) ) dw = 4;
          if ( varname[0] == 'T' && !strcmp(varname, "TIME"  ) ) dw = 5;
          if ( varname[0] == 'H' && !strcmp(varname, "HOUR"  ) ) dw = 6;
          if ( varname[0] == 'M' && !strcmp(varname, "MINUTE") ) dw = 7;
          if ( varname[0] == 'S' && !strcmp(varname, "SECOND") ) dw = 8;
		  //* en
		  if ( varname[0] == 'I' && !strcmp(varname, "INPUT" ) ) dw = 9;
		  //*/en
          if (dw > 0) 
		  {
			  if(dw < 9)
			  {
				SYSTEMTIME st;
				char DateString[64];
				GetLocalTime(&st); 
				switch (dw) {
		            case 1: sprintf(DateString, "%02d-%02d-%d", st.wDay, st.wMonth, st.wYear); break;
					case 2: sprintf(DateString, "%d", st.wYear); break;
					case 3: sprintf(DateString, "%02d", st.wMonth); break;
					case 4: sprintf(DateString, "%02d", st.wDay); break;
					case 5: sprintf(DateString, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond); break;
					case 6: sprintf(DateString, "%02d", st.wHour); break;
					case 7: sprintf(DateString, "%02d", st.wMinute); break;
					case 8: sprintf(DateString, "%02d", st.wSecond); break;
					//case 9: strcpy(DateString, editStr);break;
					default: break;
				}
              
	          strcpy(result, DateString);
              result+=strlen(DateString);
			  }
			  else
				  if(dw==9)
				  {
				      strcpy(result, editStr);
		              result+=strlen(editStr);
				  }
			  arg+=counter+varlen;
					  
//vls-end//

          } else {
                VAR_INDEX ind = VarList.find(varname);
                if( ind != VarList.end() ) {
                    VAR* pvar = ind->second;
	                strcpy(result, (char*)pvar->m_strVal.data());
                    result+=pvar->m_strVal.length();
                    arg+=counter+varlen;
                } else {
                    strncpy(result,arg,counter+varlen);
                    result+=varlen+counter;
                    arg+=varlen+counter;
                }
          }
      }
      else  {  
          strncpy(result,arg,counter+varlen);
          result+=varlen+counter;
          arg+=varlen+counter;
      }
    }
    else if (*arg==DEFAULT_OPEN) {
      nest++;
      *result++= *arg++;
    }
    else if (*arg==DEFAULT_CLOSE) {
      nest--;
      *result++= *arg++;
    }
    else if (*arg=='\\' && *(arg+1)=='$' && nest==0) {
      arg++;
      *result++= *arg++;
    }
       
    else
      *result++= *arg++;
  }
  *result='\0';
}

/*************************/
/* the #tolower command */
/*************************/
void tolower_command(char *arg)
{
    char left[BUFFER_SIZE], right[BUFFER_SIZE], arg2[BUFFER_SIZE];

    arg = get_arg_in_braces(arg, left,  STOP_SPACES);
    arg = get_arg_in_braces(arg, right, WITH_SPACES);
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
          sprintf(arg2, rs::rs(1222),left, right);
          tintin_puts2(arg2);
        }
    }
}

/*************************/
/* the #toupper command */
/*************************/
void toupper_command(char *arg)
{
    char left[BUFFER_SIZE], right[BUFFER_SIZE], arg2[BUFFER_SIZE];

    arg = get_arg_in_braces(arg, left,  STOP_SPACES);
    arg = get_arg_in_braces(arg, right, WITH_SPACES);
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
          sprintf(arg2, rs::rs(1222),left, right);
          tintin_puts2(arg2);
        }
    }
}
