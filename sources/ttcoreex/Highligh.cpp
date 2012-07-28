/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: highlight.c - functions related to the highlight command    */
/*                             TINTIN ++                             */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                     coded by Bill Reiss 1993                      */
/*********************************************************************/
#include "stdafx.h"
#include "tintin.h"


int is_high_arg(char *s);

/***************************/
/* the #highlight command  */
/***************************/

BOOL show_high(CGROUP* pGroup)
{
    BOOL bFound = FALSE;

    HLIGHT_INDEX  ind = HlightList.begin();
    while (ind  != HlightList.end() ) {
        HLIGHT* ph = ind->second;
        if ( !pGroup || pGroup == ph->m_pGroup  ) {
            char temp[BUFFER_SIZE];
            sprintf(temp, rs::rs(1009), (char*)ind->first.data(), (char*)(ph->m_strColor.data()), (char*)ph->m_pGroup->m_strName.data());
            tintin_puts2(temp);
            bFound = TRUE;
        }
        ind++;
    }

    return bFound;
}


void parse_high(char *arg)
{
    char left[BUFFER_SIZE], right[BUFFER_SIZE], gname[BUFFER_SIZE],  result[BUFFER_SIZE];
    int colflag = TRUE;
  
    arg=get_arg_in_braces(arg, left, STOP_SPACES);
    if(!*left) {
        tintin_puts2(rs::rs(1064));
        show_high();
        return;
    }  
    arg=get_arg_in_braces(arg, right, WITH_SPACES);

    if ( !*right ) {
        tintin_puts2(rs::rs(1065));
        return;
    }

    arg=get_arg_in_braces(arg, gname, STOP_SPACES);


    HLIGHT_INDEX ind = HlightList.find(right);
    HLIGHT* ph;
    BOOL bSetColor = TRUE;
    
    if ( ind != HlightList.end() ) {
        ph = ind->second;
        if ( ph->SetColor(left) ) {
            if ( *gname ) 
                ph->SetGroup(gname);
        }
        else 
            bSetColor = FALSE;
    } else {
        ph = new HLIGHT;
        if ( ph->SetColor(left) ) {
            ph->SetGroup (gname);
            ph->m_strPattern = right;
            HlightList[right] = ph;
        } else {
            bSetColor = FALSE;
            delete ph;
        }
    }
    if ( !bSetColor ) {
        tintin_puts2(rs::rs(1066));
        tintin_puts2(rs::rs(1067));
        tintin_puts2(rs::rs(1068));
        tintin_puts2(rs::rs(1069));
        tintin_puts2(rs::rs(1070));
        tintin_puts2(rs::rs(1071));
    } else {
        if (mesvar[MSG_HIGH]) {
            sprintf(result, rs::rs(1072), right, left, (char*)ph->m_pGroup->m_strName.data());
            tintin_puts2(result);
        }
    }

}


/*****************************/
/* the #unhighlight command */
/*****************************/

void unhighlight_command(char *arg)
{
    char left[BUFFER_SIZE] ,result[BUFFER_SIZE];
    BOOL bFound = FALSE;
  
    arg=get_arg_in_braces(arg, left, WITH_SPACES);

    HLIGHT_INDEX ind = HlightList.begin();

    while (ind  != HlightList.end() ) {
        if ( match(left, (char*)ind->first.data()) ){
            HLIGHT* pal = ind->second;
            if (mesvar[MSG_HIGH]) {
                sprintf(result, rs::rs(1073), (char*)ind->first.data());
                tintin_puts2(result);
            }
            bFound = TRUE;
            HLIGHT_INDEX ind1 = ind++;
            HlightList.erase(ind1);
            delete pal;
        } else 
            ind++;
    }
  
    if (!bFound && mesvar[MSG_HIGH]) {
        sprintf(result, rs::rs(1007), left);
        tintin_puts2(result);
    }    

}

static int BG = 40;
static int FG = 37;
static BOOL bBold = FALSE;
static char ANSI[128] = DEFAULT_END_COLOR;
static void ScanForAnsi(char* ptrFirst, char* ptrLast)
{
    char* ptr = ptrFirst;
    char Text[BUFFER_SIZE];

    while ( ptr <= ptrLast ) {
        if (*ptr++ == 0x1B ) {
            if ( *ptr++ != '[' /*|| !isdigit(*src)*/ ) {
                while ( *ptr && *ptr != 'm' ) ptr++;
                if ( *ptr == 'm' )
                    ptr++;
                continue;
            }
            do {        
                // may be need skip to ; . But .... Speed
                Text[0] = 0;
                char* dest = Text;
                while ( isdigit(*ptr) && ptr <= ptrLast) 
                    *dest++ = *ptr++;
                *dest = 0;
                if ( Text[0] ) {
                    int value = atoi(Text);
                    if ( !value ) {
                        BG = 40;
                        FG = 37;
                        bBold = FALSE;
                    } else 
                    if ( value == 1 ) 
                        bBold = TRUE;
                    else 
                    if ( value <= 37 && value >= 30) {
                        FG = value;
                    }else 
                    if ( value <= 47 && value >= 40) {
                        BG = value;
                    }
                }
            } while ( *ptr && *ptr++ != 'm' && ptr<= ptrLast);
        }
    }
    if ( bBold ) 
        sprintf(ANSI, "\x1B[1;%d;%dm" , BG, FG);
    else 
        sprintf(ANSI, "\x1B[0;%d;%dm" , BG, FG);
}

void do_one_high(char *line)
{
    /* struct listnode *ln, *myhighs; */
    char temp[BUFFER_SIZE],result[BUFFER_SIZE];
    char *firstch_ptr;

    HLIGHT_INDEX ind = HlightList.begin();

    while (ind  != HlightList.end() ) {
        HLIGHT* ph = ind->second;
        if(ph->m_pGroup->m_bEnabled && check_one_action(line,(char*)ind->first.data())) {
            BOOL bAnchored = FALSE;
            firstch_ptr=(char*)ind->first.data();
            if (*(firstch_ptr)=='^') {
                firstch_ptr++;
                bAnchored = TRUE;
            }
            prepare_actionalias(firstch_ptr, temp);
            int pattern_len = strlen(temp);
            if ( pattern_len == 0 ) 
                continue;

            result[0] = 0;
            if ( bAnchored ) {
                strcpy(result, (char*)ph->m_strAnsi.data());
                strcat(result,temp);
                strcat(result,DEFAULT_END_COLOR);
                strcat ( result , line+pattern_len);
                strcpy(line, result);
            } else {
                char* line1 = line;
                char* ptr, *res = result;
                while ( (ptr = strstr(line1, temp)) != NULL ) {
                    // now find last ESC command
                    ScanForAnsi(line, ptr);

                    strncpy(res, line1, ptr-line1 );
                    res += ptr-line1;
                    strcpy(res, ph->m_strAnsi.data());
                    strcat(res, temp);
                    strcat(res, ANSI);
                    res += strlen(res);
                    line1 += pattern_len + (ptr-line1);
                
                }
                if ( *line1 ) {
                    strcat(result, line1);
                }
                strcpy(line, result);

            }
            if( !bMultiHighlight ) 
                return;
        }
        ind++;
    }
    // check for last ANSI color 
    int len = strlen(line);
    if ( len ) 
        ScanForAnsi (line, line+len - 1);
}

void add_codes(char *line, char *result, char *htype, BOOL bAddTAil)
{
  char *tmp1, *tmp2, tmp3[BUFFER_SIZE];
  int code;
  sprintf(result, "%s", DEFAULT_BEGIN_COLOR);
  tmp1 = htype;
  tmp2 = tmp1;
  while (*tmp2!='\0') {
    tmp2++;
    while (*tmp2!=','&&*tmp2!='\0') tmp2++;
    while (isspace(*tmp1)) tmp1++;
    strncpy(tmp3, tmp1, tmp2-tmp1);
    tmp3[tmp2-tmp1]='\0';
    code=-1;
    if (isdigit(*tmp3)) {
      sscanf(tmp3, "%d", &code);
      code--;
    }
    if (is_abrev(tmp3, "black") || code==0) strcat(result, ";30");
      else if (is_abrev(tmp3, "red") || code==1) strcat(result, ";31");
      else if (is_abrev(tmp3, "green") || code==2) strcat(result, ";32");
      else if (is_abrev(tmp3, "brown") || code==3) strcat(result, ";33");
      else if (is_abrev(tmp3, "blue") || code==4) strcat(result, ";34");
      else if (is_abrev(tmp3, "magenta") || code==5) strcat(result, ";35");
      else if (is_abrev(tmp3, "cyan") || code==6) strcat(result, ";36");
      else if (is_abrev(tmp3, "grey") || code==7) strcat(result, ";37");
      else if (is_abrev(tmp3, "charcoal") || code==8) strcat(result, ";30;1");
      else if (is_abrev(tmp3, "light red") || code==9) strcat(result, ";31;1");
      else if (is_abrev(tmp3, "light green") || code==10) strcat(result, ";32;1");
      else if (is_abrev(tmp3, "yellow") || code==11) strcat(result, ";33;1");
      else if (is_abrev(tmp3, "light blue") || code==12) strcat(result, ";34;1");
      else if (is_abrev(tmp3, "light magenta") || code==13) strcat(result, ";35;1");
      else if (is_abrev(tmp3, "light cyan")|| code==14) strcat(result, ";36;1");
      else if (is_abrev(tmp3, "white") || code==15) strcat(result, ";37;1");
      else if (is_abrev(tmp3, "b black") || code==16) strcat(result, ";40");
      else if (is_abrev(tmp3, "b red") || code==17) strcat(result, ";41");
      else if (is_abrev(tmp3, "b green") || code==18) strcat(result, ";42");
      else if (is_abrev(tmp3, "b brown") || code==19) strcat(result, ";43");
      else if (is_abrev(tmp3, "b blue") || code==20) strcat(result, ";44");
      else if (is_abrev(tmp3, "b magenta") || code==21) strcat(result, ";45");
      else if (is_abrev(tmp3, "b cyan") || code==22) strcat(result, ";46");
      else if (is_abrev(tmp3, "b grey") || code==23) strcat(result, ";47");
      else if (is_abrev(tmp3, "b charcoal") || code==24) strcat(result, ";40;1");
      else if (is_abrev(tmp3, "b light red") || code==25) strcat(result, ";41;1");
      else if (is_abrev(tmp3, "b light green") || code==26) strcat(result, ";42;1");
      else if (is_abrev(tmp3, "b yellow") || code==27) strcat(result, ";43;1");
      else if (is_abrev(tmp3, "b light blue") || code==28) strcat(result, ";44;1");
      else if (is_abrev(tmp3, "b light magenta") || code==29) strcat(result, ";45;1");
      else if (is_abrev(tmp3, "b light cyan") || code==30) strcat(result, ";46;1");
      else if (is_abrev(tmp3, "b white") || code==31) strcat(result, ";47;1");
      else if (is_abrev(tmp3, "bold")) strcat(result, ";1");
      else if (is_abrev(tmp3, "faint")) strcat(result, ";2");
      else if (is_abrev(tmp3, "blink")) strcat(result, ";5");
      else if (is_abrev(tmp3, "italic")) strcat(result, ";3");
      else if (is_abrev(tmp3, "reverse")) strcat(result, ";7");
    tmp1= tmp2+1;
  }
  strcat(result, "m");
  if ( bAddTAil ) {
      strcat(result, line);
      strcat(result, DEFAULT_END_COLOR);
  }
}


void DLLEXPORT RemoveHlight(char* name) 
{
    HLIGHT_INDEX ind = HlightList.find(name);
    if ( ind != HlightList.end() ) {
        delete ind->second;
        HlightList.erase(ind);
    }
}

PHLIGHT DLLEXPORT SetHlight(char* color, char* pattern, char* group) 
{
    if ( !pattern || !*pattern) 
        return NULL;
    
    HLIGHT_INDEX ind = HlightList.find(pattern);
    HLIGHT* ph;

    if ( ind != HlightList.end() ) {
        ph = ind->second;
        if ( ph->SetColor(color) ) {
            ph->SetGroup(group);
        }
    } else {
        ph = new HLIGHT;
        if ( ph->SetColor(color) ) {
            ph->SetGroup (group);
            ph->m_strPattern = pattern;
            HlightList[pattern] = ph;
        } else {
            delete ph;
            return NULL;
        }
    }
    return ph;
}

PPHLIGHT DLLEXPORT GetHlightList(int* size)
{
    *size = HlightList.size();

    HLIGHT_INDEX ind = HlightList.begin();
    int i = 0;
    while (ind != HlightList.end() ) {
        JMCObjRet[i++] = ind->second;
        ind++;
    }
    return (PPHLIGHT)JMCObjRet;
}

PHLIGHT DLLEXPORT GetHlight(char* name)
{
    HLIGHT_INDEX ind = HlightList.find(name);
    if ( ind == HlightList.end() ) 
        return NULL;
    else 
        return ind->second;

}

