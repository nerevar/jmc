#include "stdafx.h"
#include "resource.h"
#include "ttcoreex.h"
#include "tintin.h"

static const unsigned char *pcre_tables = NULL;

std::map<int, TIMER*> TIMER_LIST;
ACTIONLIST ActionList;
ALIASLIST AliasList;
GROUPLIST GroupList;
VARLIST  VarList;
HLIGHTLIST HlightList;

//vls-begin// script files
SCRIPTFILELIST ScriptFileList;
//vls-end//

TIMER::TIMER(int ID, int Interval, int PreInterval) 
{
    m_nID = ID;
    m_nInterval = Interval;
    m_nPreinterval = PreInterval;
    m_dwLastTickCount = GetTickCount();
    m_bPreTimerDone = FALSE;
}


CGROUP::CGROUP()
{
    m_bEnabled = TRUE;
    m_bGlobal = FALSE;
}

CGROUP::CGROUP(char* name, BOOL bGlobal )
{
    m_bEnabled = TRUE;
    m_bGlobal = bGlobal;
    m_strName = name;
}

CGROUP::~CGROUP()
{
    // remove all associated objects here 
    ALIAS_INDEX ind = AliasList.begin();
    while (ind  != AliasList.end() ) {
        ALIAS* pal = ind->second;
        if ( pal->m_pGroup == this ) {
            ALIAS_INDEX ind1 = ind++;
            AliasList.erase(ind1);
            delete pal;
        }else 
            ind++;
    }

    ACTION_INDEX aind = ActionList.begin();
    while ( aind != ActionList.end() ) {
        // CActionPtr pac = *aind;
        ACTION* pac = *aind;
        if ( pac->m_pGroup == this ){
            aind++;
            // pac.Clear ();
            delete pac;
            ActionList.remove(pac);
        } else 
            aind++;
    }

    HLIGHT_INDEX hind = HlightList.begin();
    while (hind  != HlightList.end() ) {
        HLIGHT* pal = hind->second;
        if ( pal->m_pGroup == this ) {
            HLIGHT_INDEX ind1 = hind++;
            HlightList.erase(ind1);
            delete pal;
        }else 
            hind++;
    }

}

ALIAS::ALIAS()
{
    m_pPcre = NULL;
    m_pExtra = NULL;
    m_bRecompile = FALSE;
    m_bDeleted = FALSE;
	m_bIgnoreCase = FALSE;
}


ALIAS::~ALIAS()
{
    if ( m_pPcre ) 
        pcre_free(m_pPcre);
    if ( m_pExtra ) 
        pcre_free(m_pExtra);
}

BOOL ALIAS::CreatePattern(char* left )
{
    if ( !left ) 
        left = (char*)m_strRegex.data ();

    if ( m_pPcre ) 
        pcre_free(m_pPcre);
    if ( m_pExtra) 
        pcre_free(m_pExtra);
    
    m_pExtra = NULL;
    m_pPcre = NULL;

	int options = 0;
	if (m_bIgnoreCase)
		options |= PCRE_CASELESS;

	if (!pcre_tables) 
		pcre_tables = pcre_maketables();

    const char* err;
    int err_offset;
    m_pPcre = pcre_compile(left, options, &err, &err_offset, pcre_tables);
    if ( !m_pPcre ) {
        std::string  errstr(rs::rs(1142));
        errstr += err;
        tintin_puts2((char*)errstr.data ());
        return FALSE;
    }
    m_pExtra = pcre_study(m_pPcre, 0 , &err);
    if ( err ) {
        std::string  errstr(rs::rs(1142));
        errstr += err;
        tintin_puts2((char*)errstr.data ());
        return FALSE;
    }
    return TRUE;
}

BOOL ALIAS::SetLeft(char* left)
{
    m_strRegex.empty();
    m_strLeft = left;
	m_bIgnoreCase = FALSE;

    if ( *left == '/' ) { // its regexp
        m_strRegex = left+1;
        int size = m_strRegex.size();
		BOOL i_flag = FALSE;
		for (int i = size - 1; i >= 0; i--) {
			if (m_strRegex[i] == 'i') {
				i_flag = TRUE;
			} else if (m_strRegex[i] == '/') {
				m_bIgnoreCase = i_flag;
				m_strRegex.resize(i);
				break;
			} else {
				break;
			}
		}
    } 

    if ( *left != '/' && strchr(left, '$' ) ) {
        m_bRecompile = TRUE;
        return TRUE;
    } else {
        return CreatePattern();
	}
}
/////

ACTION::ACTION()
{
    m_nPriority = 5;
    m_pPcre = NULL;
    m_pExtra = NULL;
    m_bRecompile = FALSE;
    m_bDeleted = FALSE;
	m_InputType = Action_TEXT;
	m_bMultiline = m_bIgnoreCase = FALSE;
}


ACTION::~ACTION()
{
    if ( m_pPcre ) 
        pcre_free(m_pPcre);
    if ( m_pExtra ) 
        pcre_free(m_pExtra);
}

BOOL ACTION::CreatePattern(char* left )
{
    if ( !left ) 
        left = (char*)m_strRegex.data ();

    if ( m_pPcre ) 
        pcre_free(m_pPcre);
    if ( m_pExtra) 
        pcre_free(m_pExtra);
    
    m_pExtra = NULL;
    m_pPcre = NULL;

	int options = 0;
	if (m_bMultiline)
		options |= PCRE_MULTILINE;
	if (m_bIgnoreCase)
		options |= PCRE_CASELESS;
	
	if (!pcre_tables) 
		pcre_tables = pcre_maketables();

    const char* err;
    int err_offset;
    m_pPcre = pcre_compile(left, options, &err, &err_offset, pcre_tables);
    if ( !m_pPcre ) {
        std::string  errstr(rs::rs(1142));
        errstr += err;
        tintin_puts2((char*)errstr.data ());
        return FALSE;
    }
    m_pExtra = pcre_study(m_pPcre, 0 , &err);
    if ( err ) {
        std::string  errstr(rs::rs(1142));
        errstr += err;
        tintin_puts2((char*)errstr.data ());
        return FALSE;
    }
    return TRUE;
}

BOOL ACTION::SetLeft(char* left)
{
    m_strRegex.empty();
    m_strLeft = left;
	m_bMultiline = m_bIgnoreCase = FALSE;

    if ( *left == '/' ) { // its regexp
        m_strRegex = left+1;
        int size = m_strRegex.size();
		BOOL i_flag = FALSE, m_flag = FALSE;
		for (int i = size - 1; i >= 0; i--) {
			if (m_strRegex[i] == 'i') {
				i_flag = TRUE;
			} else if (m_strRegex[i] == 'm') {
				m_flag = TRUE;
			} else if (m_strRegex[i] == '/') {
				m_bMultiline = m_flag;
				m_bIgnoreCase = i_flag;
				m_strRegex.resize(i);
				break;
			} else {
				break;
			}
		}
    } 

    if ( *left != '/' && strchr(left, '$' ) ) {
        m_bRecompile = TRUE;
        return TRUE;
    } else {
        return CreatePattern();
	}
}

GROUPED_NODE::GROUPED_NODE()
{
    m_pGroup = NULL;
}

void GROUPED_NODE::SetGroup(char* group )
{
    if ( !group || *group == 0 ) 
        if ( m_pGroup ) 
            return;
        else 
            group = DEFAULT_GROUP_NAME;

    GROUP_INDEX ind = GroupList.find(group);
    if ( ind != GroupList.end () ) {
        m_pGroup = ind->second;
        return;
    }

    CGROUP* pGroup = new CGROUP(group);
    GroupList[group] = pGroup;
    m_pGroup = pGroup;
}


VAR::VAR(char* val, BOOL bGlobal )
{
    if ( val ) 
        m_strVal = val;
    bGlobal= bGlobal;
}

static int is_high_arg(char *s)
{
  int code;
  sscanf(s, "%d", &code);
  if (is_abrev(s, "red") || is_abrev(s, "blue") || is_abrev(s, "cyan") ||
     is_abrev(s, "green") || is_abrev(s, "yellow") ||
     is_abrev(s, "magenta") || is_abrev(s, "white") ||
     is_abrev(s, "grey") || is_abrev(s, "black") ||
     is_abrev(s, "brown") || is_abrev(s, "charcoal") ||
     is_abrev(s, "light red") || is_abrev(s, "light blue") ||
     is_abrev(s, "light cyan") || is_abrev(s, "light magenta") ||
     is_abrev(s, "light green") || is_abrev(s, "b red") ||
     is_abrev(s, "b blue") || is_abrev(s, "b cyan") ||
     is_abrev(s, "b green") || is_abrev(s, "b yellow") ||
     is_abrev(s, "b magenta") || is_abrev(s, "b white") ||
     is_abrev(s, "b grey") || is_abrev(s, "b black") ||
     is_abrev(s, "b brown") || is_abrev(s, "b charcoal") ||
     is_abrev(s, "b light red") || is_abrev(s, "b light blue") ||
     is_abrev(s, "b light cyan") || is_abrev(s, "b light magenta") ||
     is_abrev(s, "b light green") || is_abrev(s, "bold") ||
     is_abrev(s, "faint") || is_abrev(s, "blink") ||
     is_abrev(s, "italic") || is_abrev(s, "reverse") || 
     (isdigit(*s) && code<33 && code>0)) return TRUE;
     else return FALSE;
}


BOOL HLIGHT::SetColor(char* color)
{
/*    if ( !is_high_arg(color) ) 
        return FALSE;
*/
    char buff[BUFFER_SIZE];
    add_codes(NULL, buff, color, FALSE);
    m_strAnsi = buff;
    m_strColor = color;
    return TRUE;
}

