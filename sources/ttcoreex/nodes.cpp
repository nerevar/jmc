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

VARTOPCRE VarPcreDeps;

//vls-begin// script files
SCRIPTFILELIST ScriptFileList;
//vls-end//


static std::list<std::string> extract_varnames(const char *line) 
{
	std::list<std::string> ret;

	while (line = strchr(line, '$')) {
		std::string varname = "";
		line++;
		while (is_allowed_symbol(line[0])) {
			varname += line[0];
			line++;
		}
		if (varname.length() > 0)
			ret.push_back(varname);
	}

	return ret;
}

static int add_dependencies(CPCRE *pPcre, const std::string line)
{
	std::list<std::string> varnames = extract_varnames(line.c_str());

	for (std::list<std::string>::iterator it = varnames.begin(); it != varnames.end(); it++)
		VarPcreDeps[*it].insert(pPcre);

	return varnames.size();
}

static void remove_dependencies(CPCRE *pPcre)
{
	for (VARTOPCRE::iterator it = VarPcreDeps.begin(); it != VarPcreDeps.end(); it++)
		it->second.erase(pPcre);
}

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

CPCRE::CPCRE()
{
	m_strSource = "";
	m_pPcre = NULL;
	m_pExtra = NULL;
	m_bContainVars = FALSE;
	m_bMultiline = m_bIgnoreCase = FALSE;
}

CPCRE::~CPCRE()
{
	remove_dependencies(this);
	Clear(TRUE);
}

void CPCRE::Clear(BOOL ResetSource)
{
	if ( m_pPcre ) {
        pcre_free(m_pPcre);
		m_pPcre = NULL;
	}
    if ( m_pExtra ) {
        pcre_free(m_pExtra);
		m_pExtra = NULL;
	}

	if ( ResetSource ) {
		m_strSource = "";
		m_bContainVars = FALSE;
		m_bMultiline = m_bIgnoreCase = FALSE;
	}
}

BOOL CPCRE::SetSource(const std::string Source, BOOL Multiline, BOOL IgnoreCase)
{
	Clear(TRUE);

	m_strSource = Source;
	m_bMultiline = Multiline;
	m_bIgnoreCase = IgnoreCase;

	m_bContainVars = (add_dependencies(this, m_strSource) ? TRUE : FALSE);
	return Recompile();
}

BOOL CPCRE::Recompile(const char *Pattern)
{
	if ( !Pattern ) 
        Pattern = (char*)m_strSource.data ();

    Clear(FALSE);

	int options = 0;
	if (m_bMultiline)
		options |= PCRE_MULTILINE;
	if (m_bIgnoreCase)
		options |= PCRE_CASELESS;
	
	if (!pcre_tables) 
		pcre_tables = pcre_maketables();

	char expression[BUFFER_SIZE];
	if (m_bContainVars) {
		prepare_actionalias((char *)Pattern, expression, sizeof(expression));
		Pattern = expression;
	}

    const char* err;
    int err_offset;
    m_pPcre = pcre_compile(Pattern, options, &err, &err_offset, pcre_tables);
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

ALIAS::ALIAS()
{
    m_bRecompile = FALSE;
    m_bDeleted = FALSE;
}


ALIAS::~ALIAS()
{
}

BOOL ALIAS::SetLeft(char* left)
{
    m_PCRE.Clear(TRUE);

    m_strLeft = left;
	BOOL i_flag = FALSE;
	std::string regexp = "";

    if ( *left == '/' ) {
		regexp = left + 1;
        
        int size = regexp.size();
		
		for (int i = size - 1; i >= 0; i--) {
			if (regexp[i] == 'i') {
				size--;
				i_flag = TRUE;
			} else if (regexp[i] == '/') {
				size--;
				break;
			} else {
				size = regexp.size();
				i_flag = FALSE;
				break;
			}
		}
		regexp.resize(size);
    } 

	if (regexp.size() > 0) {
        return m_PCRE.SetSource(regexp, FALSE, i_flag);
	}
	
	if (strchr(left, '$' ))
        m_bRecompile = TRUE;
	return TRUE;
}
/////

ACTION::ACTION()
{
    m_nPriority = 5;
    m_bRecompile = FALSE;
    m_bDeleted = FALSE;
	m_InputType = Action_TEXT;
	m_bGlobal = FALSE;
}


ACTION::~ACTION()
{
}

BOOL ACTION::SetLeft(char* left)
{
    m_PCRE.Clear(TRUE);

    m_strLeft = left;
	BOOL i_flag = FALSE, m_flag = FALSE, g_flag = FALSE;
	std::string regexp = "";

    if ( *left == '/' ) {
		regexp = left + 1;
        
        int size = regexp.size();
		
		for (int i = size - 1; i >= 0; i--) {
			if (regexp[i] == 'i') {
				size--;
				i_flag = TRUE;
			} else if (regexp[i] == 'm') {
				size--;
				m_flag = TRUE;
			} else if (regexp[i] == 'g') {
				size--;
				g_flag = TRUE;
			} else if (regexp[i] == '/') {
				size--;
				break;
			} else {
				size = regexp.size();
				i_flag = m_flag = g_flag = FALSE;
				break;
			}
		}
		regexp.resize(size);
    } 

	m_bGlobal = g_flag;

	if (regexp.size() > 0) {
        return m_PCRE.SetSource(regexp, m_flag, i_flag);
	}
	
	if (strchr(left, '$' ))
        m_bRecompile = TRUE;
	return TRUE;
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

