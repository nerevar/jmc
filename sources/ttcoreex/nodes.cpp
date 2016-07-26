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

static std::list<std::wstring> extract_varnames(const wchar_t *line) 
{
	std::list<std::wstring> ret;

	while (line = wcschr(line, L'$')) {
		std::wstring varname = L"";
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

static int add_dependencies(CPCRE *pPcre, const std::wstring line)
{
	std::list<std::wstring> varnames = extract_varnames(line.c_str());

	for (std::list<std::wstring>::iterator it = varnames.begin(); it != varnames.end(); it++)
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

CGROUP::CGROUP(const wchar_t* name, BOOL bGlobal )
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
	m_strSource = L"";
	m_dwOptions = 0;
	m_pPcre = NULL;
	m_pExtra = NULL;
	m_bContainVars = FALSE;
}

CPCRE::~CPCRE()
{
	remove_dependencies(this);
	Clear(TRUE);
}

void CPCRE::Clear(BOOL ResetSource)
{
	if (m_pPcre) {
		pcre16_free(m_pPcre);
		pcre16_free(m_pExtra);
		m_pPcre = NULL;
		m_pExtra = NULL;
	}

	if (ResetSource) {
		m_strSource = L"";
		m_dwOptions = 0;
		m_bContainVars = FALSE;
	}
}

BOOL CPCRE::SetSource(const std::wstring Source, BOOL Multiline, BOOL IgnoreCase)
{
	Clear(TRUE);

	m_strSource = Source;
	m_dwOptions = PCRE_UTF16 | PCRE_UCP;
	if (Multiline)
		m_dwOptions |= PCRE_MULTILINE | PCRE_DOTALL;
	if (IgnoreCase)
		m_dwOptions |= PCRE_CASELESS;

	m_bContainVars = (add_dependencies(this, m_strSource) ? TRUE : FALSE);
	return Recompile();
}

BOOL CPCRE::Recompile()
{
	USES_CONVERSION;

	const wchar_t *pattern = m_strSource.c_str();

    Clear(FALSE);

	wchar_t expression[BUFFER_SIZE];
	if (m_bContainVars) {
		prepare_actionalias(pattern, expression, sizeof(expression)/sizeof(wchar_t));
		pattern = expression;
	}

	m_pPcre = NULL;
	m_pExtra = NULL;

	if (!pcre_tables) 
		pcre_tables = pcre16_maketables();

    const char* err;
    int err_offset;
    m_pPcre = pcre16_compile(pattern, m_dwOptions, &err, &err_offset, pcre_tables);
    if (!m_pPcre) {
        std::wstring  errstr(rs::rs(1142));
        errstr += A2W(err);
        tintin_puts2(errstr.c_str());
        return FALSE;
    }
    m_pExtra = pcre16_study(m_pPcre, 0 , &err);
    if (err) {
        std::wstring  errstr(rs::rs(1142));
        errstr += A2W(err);
        tintin_puts2(errstr.c_str());
		pcre16_free(m_pPcre);
		if (m_pExtra)
			pcre16_free(m_pExtra);
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

BOOL ALIAS::SetLeft(const wchar_t* left)
{
    m_PCRE.Clear(TRUE);

    m_strLeft = left;
	BOOL i_flag = FALSE;
	std::wstring regexp = L"";

    if ( *left == L'/' ) {
		regexp = left + 1;
        
        int size = regexp.size();
		
		for (int i = size - 1; i >= 0; i--) {
			if (regexp[i] == L'i') {
				size--;
				i_flag = TRUE;
			} else if (regexp[i] == L'/') {
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
	
	if (wcschr(left, L'$' ))
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

BOOL ACTION::SetLeft(const wchar_t* left)
{
    m_PCRE.Clear(TRUE);

    m_strLeft = left;
	BOOL i_flag = FALSE, m_flag = FALSE, g_flag = FALSE;
	std::wstring regexp = L"";

    if ( *left == L'/' ) {
		regexp = left + 1;
        
        int size = regexp.size();
		
		for (int i = size - 1; i >= 0; i--) {
			if (regexp[i] == L'i') {
				size--;
				i_flag = TRUE;
			} else if (regexp[i] == L'm') {
				size--;
				m_flag = TRUE;
			} else if (regexp[i] == L'g') {
				size--;
				g_flag = TRUE;
			} else if (regexp[i] == L'/') {
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
	
	if (wcschr(left, L'$' ))
        m_bRecompile = TRUE;
	return TRUE;
}

GROUPED_NODE::GROUPED_NODE()
{
    m_pGroup = NULL;
}

void GROUPED_NODE::SetGroup(const wchar_t* group )
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


VAR::VAR(const wchar_t* val, BOOL bGlobal )
{
    if ( val ) 
        m_strVal = val;
    bGlobal= bGlobal;
}

static int is_high_arg(const wchar_t *s)
{
  int code;
  swscanf(s, L"%d", &code);
  if (is_abrev(s, L"red") || is_abrev(s, L"blue") || is_abrev(s, L"cyan") ||
     is_abrev(s, L"green") || is_abrev(s, L"yellow") ||
     is_abrev(s, L"magenta") || is_abrev(s, L"white") ||
     is_abrev(s, L"grey") || is_abrev(s, L"black") ||
     is_abrev(s, L"brown") || is_abrev(s, L"charcoal") ||
     is_abrev(s, L"light red") || is_abrev(s, L"light blue") ||
     is_abrev(s, L"light cyan") || is_abrev(s, L"light magenta") ||
     is_abrev(s, L"light green") || is_abrev(s, L"b red") ||
     is_abrev(s, L"b blue") || is_abrev(s, L"b cyan") ||
     is_abrev(s, L"b green") || is_abrev(s, L"b yellow") ||
     is_abrev(s, L"b magenta") || is_abrev(s, L"b white") ||
     is_abrev(s, L"b grey") || is_abrev(s, L"b black") ||
     is_abrev(s, L"b brown") || is_abrev(s, L"b charcoal") ||
     is_abrev(s, L"b light red") || is_abrev(s, L"b light blue") ||
     is_abrev(s, L"b light cyan") || is_abrev(s, L"b light magenta") ||
     is_abrev(s, L"b light green") || is_abrev(s, L"bold") ||
     is_abrev(s, L"faint") || is_abrev(s, L"blink") ||
     is_abrev(s, L"italic") || is_abrev(s, L"reverse") || 
     (iswdigit(*s) && code<33 && code>0)) return TRUE;
     else return FALSE;
}


BOOL HLIGHT::SetColor(const wchar_t* color)
{
/*    if ( !is_high_arg(color) ) 
        return FALSE;
*/
    wchar_t buff[BUFFER_SIZE];
    add_codes(NULL, buff, color, FALSE);
    m_strAnsi = buff;
    m_strColor = color;
    return TRUE;
}

