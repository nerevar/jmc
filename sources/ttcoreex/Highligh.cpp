/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: highlight.c - functions related to the highlight command    */
/*                             TINTIN ++                             */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                     coded by Bill Reiss 1993                      */
/*********************************************************************/
#include "stdafx.h"
#include "tintin.h"


int is_high_arg(wchar_t *s);

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
            wchar_t temp[BUFFER_SIZE];
            swprintf(temp, rs::rs(1009), ind->first.c_str(), ph->m_strColor.c_str(), ph->m_pGroup->m_strName.c_str());
            tintin_puts2(temp);
            bFound = TRUE;
        }
        ind++;
    }

    return bFound;
}


void parse_high(wchar_t *arg)
{
    wchar_t left[BUFFER_SIZE], right[BUFFER_SIZE], gname[BUFFER_SIZE],  result[BUFFER_SIZE];
    int colflag = TRUE;
  
    arg=get_arg_in_braces(arg,left,STOP_SPACES,sizeof(left)/sizeof(wchar_t)-1);
    if(!*left) {
        tintin_puts2(rs::rs(1064));
        show_high();
        return;
    }  
    arg=get_arg_in_braces(arg,right,WITH_SPACES,sizeof(right)/sizeof(wchar_t)-1);

    if ( !*right ) {
        tintin_puts2(rs::rs(1065));
        return;
    }

    arg=get_arg_in_braces(arg,gname,STOP_SPACES,sizeof(gname)/sizeof(wchar_t)-1);


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
        ph = new HLIGHT();
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
            swprintf(result, rs::rs(1072), right, left, ph->m_pGroup->m_strName.c_str());
            tintin_puts2(result);
        }
    }

}


/*****************************/
/* the #unhighlight command */
/*****************************/

void unhighlight_command(wchar_t *arg)
{
    wchar_t left[BUFFER_SIZE] ,result[BUFFER_SIZE];
    BOOL bFound = FALSE;
  
    arg=get_arg_in_braces(arg,left,WITH_SPACES,sizeof(left)/sizeof(wchar_t)-1);

    HLIGHT_INDEX ind = HlightList.begin();

    while (ind  != HlightList.end() ) {
        if ( match(left, ind->first.c_str()) ){
            HLIGHT* pal = ind->second;
            if (mesvar[MSG_HIGH]) {
                swprintf(result, rs::rs(1073), ind->first.c_str());
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
        swprintf(result, rs::rs(1007), left);
        tintin_puts2(result);
    }    

}

static int BG = 40;
static int FG = 37;
static BOOL bBold = FALSE;
static wchar_t ANSI[128] = DEFAULT_END_COLOR;
static void ScanForAnsi(wchar_t* ptrFirst, wchar_t* ptrLast)
{
    wchar_t* ptr = ptrFirst;
    wchar_t Text[BUFFER_SIZE];

    while ( ptr <= ptrLast ) {
        if (*ptr++ == ANSI_COMMAND_CHAR ) {
            if ( *ptr++ != L'[' /*|| !isdigit(*src)*/ ) {
                while ( *ptr && *ptr != L'm' ) ptr++;
                if ( *ptr == 'm' )
                    ptr++;
                continue;
            }
            do {        
                // may be need skip to ; . But .... Speed
                Text[0] = 0;
                wchar_t* dest = Text;
                while ( iswdigit(*ptr) && ptr <= ptrLast) 
                    *dest++ = *ptr++;
                *dest = 0;
                if ( Text[0] ) {
                    int value = _wtoi(Text);
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
        swprintf(ANSI, L"%c[1;%d;%dm" , ANSI_COMMAND_CHAR, BG, FG);
    else 
        swprintf(ANSI, L"%c[0;%d;%dm" , ANSI_COMMAND_CHAR, BG, FG);
}

void do_one_high(wchar_t *line)
{
    /* struct listnode *ln, *myhighs; */
    wchar_t temp[BUFFER_SIZE],result[BUFFER_SIZE];
    const wchar_t *firstch_ptr;

    HLIGHT_INDEX ind = HlightList.begin();

    while (ind  != HlightList.end() ) {
        HLIGHT* ph = ind->second;
        if(ph->m_pGroup->m_bEnabled && check_one_action(line,ind->first.c_str())) {
            BOOL bAnchored = FALSE;
            firstch_ptr=ind->first.c_str();
            if (*(firstch_ptr)==L'^') {
                firstch_ptr++;
                bAnchored = TRUE;
            }
            prepare_actionalias(firstch_ptr, temp, sizeof(temp)/sizeof(wchar_t));
            int pattern_len = wcslen(temp);
            if ( pattern_len == 0 ) 
                continue;

            result[0] = 0;
            if ( bAnchored ) {
                wcscpy(result, ph->m_strAnsi.c_str());
                wcscat(result,temp);
                wcscat(result,DEFAULT_END_COLOR);
                wcscat ( result , line+pattern_len);
                wcscpy(line, result);
            } else {
                wchar_t* line1 = line;
                wchar_t* ptr, *res = result;
                while ( (ptr = wcsstr(line1, temp)) != NULL ) {
                    // now find last ESC command
                    ScanForAnsi(line, ptr);

                    wcsncpy(res, line1, ptr-line1 );
                    res += ptr-line1;
                    wcscpy(res, ph->m_strAnsi.data());
                    wcscat(res, temp);
                    wcscat(res, ANSI);
                    res += wcslen(res);
                    line1 += pattern_len + (ptr-line1);
                
                }
                if ( *line1 ) {
                    wcscat(result, line1);
                }
                wcscpy(line, result);

            }
            if( !bMultiHighlight ) 
                return;
        }
        ind++;
    }
    // check for last ANSI color 
    int len = wcslen(line);
    if ( len ) 
        ScanForAnsi (line, line+len - 1);
}

void add_codes(const wchar_t *line, wchar_t *result, const wchar_t *htype, BOOL bAddTAil)
{
  const wchar_t *tmp1, *tmp2;
  wchar_t tmp3[BUFFER_SIZE];
  int code;
  swprintf(result, L"%s", DEFAULT_BEGIN_COLOR);
  tmp1 = htype;
  tmp2 = tmp1;
  while (*tmp2!=L'\0') {
    tmp2++;
    while (*tmp2!=L','&&*tmp2!=L'\0') tmp2++;
    while (iswspace(*tmp1)) tmp1++;
    wcsncpy(tmp3, tmp1, tmp2-tmp1);
    tmp3[tmp2-tmp1]=L'\0';
    code=-1;
    if (iswdigit(*tmp3)) {
      swscanf(tmp3, L"%d", &code);
      code--;
    }
    if (is_abrev(tmp3, L"black") || code==0) wcscat(result, L";30");
      else if (is_abrev(tmp3, L"red") || code==1) wcscat(result, L";31");
      else if (is_abrev(tmp3, L"green") || code==2) wcscat(result, L";32");
      else if (is_abrev(tmp3, L"brown") || code==3) wcscat(result, L";33");
      else if (is_abrev(tmp3, L"blue") || code==4) wcscat(result, L";34");
      else if (is_abrev(tmp3, L"magenta") || code==5) wcscat(result, L";35");
      else if (is_abrev(tmp3, L"cyan") || code==6) wcscat(result, L";36");
      else if (is_abrev(tmp3, L"grey") || code==7) wcscat(result, L";37");
      else if (is_abrev(tmp3, L"charcoal") || code==8) wcscat(result, L";30;1");
      else if (is_abrev(tmp3, L"light red") || code==9) wcscat(result, L";31;1");
      else if (is_abrev(tmp3, L"light green") || code==10) wcscat(result, L";32;1");
      else if (is_abrev(tmp3, L"yellow") || code==11) wcscat(result, L";33;1");
      else if (is_abrev(tmp3, L"light blue") || code==12) wcscat(result, L";34;1");
      else if (is_abrev(tmp3, L"light magenta") || code==13) wcscat(result, L";35;1");
      else if (is_abrev(tmp3, L"light cyan")|| code==14) wcscat(result, L";36;1");
      else if (is_abrev(tmp3, L"white") || code==15) wcscat(result, L";37;1");
      else if (is_abrev(tmp3, L"b black") || code==16) wcscat(result, L";40");
      else if (is_abrev(tmp3, L"b red") || code==17) wcscat(result, L";41");
      else if (is_abrev(tmp3, L"b green") || code==18) wcscat(result, L";42");
      else if (is_abrev(tmp3, L"b brown") || code==19) wcscat(result, L";43");
      else if (is_abrev(tmp3, L"b blue") || code==20) wcscat(result, L";44");
      else if (is_abrev(tmp3, L"b magenta") || code==21) wcscat(result, L";45");
      else if (is_abrev(tmp3, L"b cyan") || code==22) wcscat(result, L";46");
      else if (is_abrev(tmp3, L"b grey") || code==23) wcscat(result, L";47");
      else if (is_abrev(tmp3, L"b charcoal") || code==24) wcscat(result, L";40;1");
      else if (is_abrev(tmp3, L"b light red") || code==25) wcscat(result, L";41;1");
      else if (is_abrev(tmp3, L"b light green") || code==26) wcscat(result, L";42;1");
      else if (is_abrev(tmp3, L"b yellow") || code==27) wcscat(result, L";43;1");
      else if (is_abrev(tmp3, L"b light blue") || code==28) wcscat(result, L";44;1");
      else if (is_abrev(tmp3, L"b light magenta") || code==29) wcscat(result, L";45;1");
      else if (is_abrev(tmp3, L"b light cyan") || code==30) wcscat(result, L";46;1");
      else if (is_abrev(tmp3, L"b white") || code==31) wcscat(result, L";47;1");
      else if (is_abrev(tmp3, L"bold")) wcscat(result, L";1");
      else if (is_abrev(tmp3, L"faint")) wcscat(result, L";2");
      else if (is_abrev(tmp3, L"blink")) wcscat(result, L";5");
      else if (is_abrev(tmp3, L"italic")) wcscat(result, L";3");
      else if (is_abrev(tmp3, L"reverse")) wcscat(result, L";7");
    tmp1= tmp2+1;
  }
  wcscat(result, L"m");
  if ( bAddTAil ) {
      wcscat(result, line);
      wcscat(result, DEFAULT_END_COLOR);
  }
}


void DLLEXPORT RemoveHlight(const wchar_t* name) 
{
    HLIGHT_INDEX ind = HlightList.find(name);
    if ( ind != HlightList.end() ) {
        delete ind->second;
        HlightList.erase(ind);
    }
}

PHLIGHT DLLEXPORT SetHlight(const wchar_t* color, const wchar_t* pattern, const wchar_t* group) 
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
        ph = new HLIGHT();
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

PHLIGHT DLLEXPORT GetHlight(const wchar_t* name)
{
    HLIGHT_INDEX ind = HlightList.find(name);
    if ( ind == HlightList.end() ) 
        return NULL;
    else 
        return ind->second;

}

