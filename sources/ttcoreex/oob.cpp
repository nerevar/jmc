#include "stdafx.h"
#include "tintin.h"
#include "telnet.h"
#include "ttcoreex.h"
#include "JmcObj.h"
#include "Utils.h"

#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

class COobObject
{
private:
	COobObject *m_pParent;
	wstring		m_Name;

	wstring		m_strVal;
	vector<COobObject *> m_listVal;
	map<wstring,COobObject*> m_dictVal;

public:
	COobObject *parent() const;

	COobObject *child(const wchar_t *Name, bool CreateIfNotExists);
	COobObject *child(int Index, bool CreateIfNotExists);

	wstring name() const;
	wstring value(bool WithQuotes) const;
	wstring key(int index) const;
	int     childrenCount() const;

	void clear();

	wchar_t *addGMCP(const wchar_t *GMCP, bool WithChildName, bool RemoveOldContents);

public:
	COobObject(COobObject *pParent, const wchar_t *Name = L"");
	~COobObject();
};

COobObject::COobObject(COobObject *pParent, const wchar_t *Name):
		m_pParent(pParent)
{
	m_Name = Name;
	transform(m_Name.begin(), m_Name.end(), m_Name.begin(), towlower);
}

static wstring scan_string_val(const wchar_t *input, wchar_t **output)
{
	wchar_t *src = (wchar_t*)input;
	wstring ret = L"";
	if (src[0] == L'"') {
		src++;
		while (src[0]) {
			if (src[0] == L'"') {
				src++;
				break;
			}
			if (src[0] == L'\\' && src[1])
				src++;
			ret += src[0];
			src++;
		}
	} else {
		bool only_digits = true, seen_dot = false;
		while (iswalpha(src[0]) || iswdigit(src[0]) || src[0] == L'-' || src[0] == L'.') {
			if (!iswdigit(src[0]))
				only_digits = false;
			if (src[0] == L'.') {
				if (!only_digits || seen_dot)
					break;
				seen_dot = true;
			}
			ret += src[0];
			src++;
		}
	}
	if (output)
		*output = src;
	return ret;
}

wchar_t * COobObject::addGMCP(const wchar_t *GMCP, bool WithChildName, bool RemoveOldContents)
{
	wchar_t *src = space_out((wchar_t*)GMCP);

	if (WithChildName) {
		wstring name = scan_string_val(src, &src);
		
		src = space_out(src);
		bool subnamed;
		if (src[0] == L'.' || src[0] == L'_' || src[0] == L'-') {
			subnamed = true;
			src++;
		} else {
			subnamed = false;
			src = space_out(src);
			if (src[0] == L':')
				src++;
		}
		src = space_out(src);

		if (name.length() > 0 && iswdigit(name[0]))
			name = L'D' + name;
		COobObject *pChild = child(name.c_str(), true);
		return pChild->addGMCP(src, subnamed, RemoveOldContents);
	}

	switch (src[0]) {
	case L'[':
		{
			src++;
			src = space_out(src);

			//if (RemoveOldContents) -- always replace arrays
			for (vector<COobObject*>::iterator it1 = m_listVal.begin(); it1 != m_listVal.end(); it1++)
				delete (*it1);
			m_listVal.clear();

			int index = 0;
			while (src[0] && src[0] != L']') {
				COobObject *pChild = child(index++, true);
				src = pChild->addGMCP(src, false, RemoveOldContents);

				src = space_out(src);
				if (src[0] == L',') {
					src++;
					src = space_out(src);
				}
			}
			if (src[0] == L']') {
				src++;
				src = space_out(src);
			}
		}
		break;
	case L'{':
		{
			src++;
			src = space_out(src);

			if (RemoveOldContents) {
				for (map<wstring,COobObject*>::iterator it2 = m_dictVal.begin(); it2 != m_dictVal.end(); it2++)
					delete it2->second;
				m_dictVal.clear();
			}

			while (src[0] && src[0] != L'}') {
				COobObject *pChild = NULL;

				wstring name = scan_string_val(src, &src);
				if (name.length() > 0 && iswdigit(name[0]))
					name = L'D' + name;
				
				src = space_out(src);
				switch (src[0]) {
				case L':':
					src++;
					src = space_out(src);
					pChild = child(name.c_str(), true);
					src = pChild->addGMCP(src, false, RemoveOldContents);
					break;
				case L'.':
				case L'_':
				case L'-':
					src++;
					pChild = child(name.c_str(), true);
					src = pChild->addGMCP(src, true, RemoveOldContents);
					break;
				default:
					//format error!
					src++;
				}
				
				src = space_out(src);
				if (src[0] == L',') {
					src++;
					src = space_out(src);
				}
			}
			if (src[0] == L'}') {
				src++;
				src = space_out(src);
			}
		}
		break;
	case L'"':
		m_strVal = scan_string_val(src, &src);
		break;
	default:
		m_strVal = scan_string_val(src, &src);
		break;
	}

	return src;
}

void COobObject::clear()
{
	m_Name = L"";
	m_strVal = L"";
	for (vector<COobObject*>::iterator it1 = m_listVal.begin(); it1 != m_listVal.end(); it1++)
		delete (*it1);
	m_listVal.clear();
	for (map<wstring,COobObject*>::iterator it2 = m_dictVal.begin(); it2 != m_dictVal.end(); it2++)
		delete it2->second;
	m_dictVal.clear();
}

COobObject::~COobObject()
{
	clear();
}

COobObject *COobObject::child(int Index, bool CreateIfNotExists)
{
	if (Index < 0)
		return NULL;

	if (m_listVal.size() == 0 && !CreateIfNotExists && Index < m_dictVal.size()) {
		for (map<wstring,COobObject*>::iterator it2 = m_dictVal.begin(); it2 != m_dictVal.end(); it2++)
			if (Index-- == 0)
				return it2->second;
	}

	if (Index < m_listVal.size())
		return m_listVal[Index];

	if (!CreateIfNotExists)
		return NULL;
	
	COobObject *pChild;
	do {
		pChild = new COobObject(this);
		m_listVal.push_back(pChild);
	} while (m_listVal.size() <= Index);
	
	return pChild;
}


COobObject *COobObject::child(const wchar_t *Name, bool CreateIfNotExists)
{
	map<wstring,COobObject*>::iterator it;
	
	wstring lower_name = Name;
	transform(lower_name.begin(), lower_name.end(), lower_name.begin(), towlower);
	
	if ((it = m_dictVal.find(lower_name)) != m_dictVal.end())
		return it->second;

	if (!CreateIfNotExists)
		return NULL;
	
	COobObject *pChild = new COobObject(this, lower_name.c_str());
	m_dictVal[lower_name] = pChild;
	
	return pChild;
}

wstring COobObject::name() const
{
	return m_Name;
}

wstring COobObject::key(int index) const
{
	if (index < 0 || index >= m_dictVal.size())
		return L"";

	for (map<wstring,COobObject*>::const_iterator it2 = m_dictVal.begin(); it2 != m_dictVal.end(); it2++)
		if (index-- == 0)
			return it2->first;

	return L"";
}

wstring COobObject::value(bool WithQuotes) const
{
	wstring ret = L"";

	if (m_strVal.length() > 0) {
		ret = m_strVal;
		if (!WithQuotes)
			return ret;

		size_t index = 0;
		for(;;) {
			if ((index = ret.find(L"\\", index)) == wstring::npos) break;
			ret.replace(index, 1, L"\\\\");
			index += 2;
		}
		index = 0;
		for(;;) {
			if ((index = ret.find(L"\"", index)) == wstring::npos) break;
			ret.replace(index, 1, L"\\\"");
			index += 2;
		}
		ret.insert(0, L'"');
		ret += L'"';
	}

	bool first;
	if (m_listVal.size() > 0) {
		ret += L'[';
		first = true;
		for (vector<COobObject*>::const_iterator it1 = m_listVal.begin(); it1 != m_listVal.end(); it1++) {
			if (!first) {
				ret += L", ";
			}
			first = false;
			ret += (*it1)->value(true);
		}
		ret += L']';
	}

	if (m_dictVal.size() > 0 || ret.length() == 0) {
		ret += L'{';
		first = true;
		for (map<wstring, COobObject*>::const_iterator it2 = m_dictVal.begin(); it2 != m_dictVal.end(); it2++) {
			if (!first) {
				ret += L", ";
			}
			first = false;
			ret += it2->second->name();
			ret += L": ";
			ret += it2->second->value(true);
		}
		ret += L'}';
	}

	return ret;
}

int COobObject::childrenCount() const
{
	return m_listVal.size() + m_dictVal.size();
}

COobObject *pRoot = NULL;
map < wstring, oob_module_info > oob_modules;

static bool initialized = false;
static void check_init()
{
	if (!initialized) {
		oob_modules[L"GMCP"] = oob_module_info();
		oob_modules[L"MSDP"] = oob_module_info();
		oob_modules[L"MSSP"] = oob_module_info();
		initialized = true;
	}
}

/*
 #oob  -- list modules (gmcp/msdp/mssp/...)
 #oob <module>  -- list enabled submodules
 #oob <module> enable|disable  -- add all or clear all known submodules
 #oob <module> add|del <submodule>  -- add/del specific submodule
 #oob <module> request <submodule>  -- request specific submodule
 */
void oob_command(wchar_t *arg)
{
	wchar_t modname[BUFFER_SIZE], command[BUFFER_SIZE], temp[BUFFER_SIZE];

	check_init();

	map< wstring, oob_module_info >::iterator it;
	set< wstring >::iterator it2;

	arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
	prepare_actionalias(temp, modname, sizeof(modname) / sizeof(wchar_t) - 1);

	bool modfound = false;
	if (modname[0]) {
		for (it = oob_modules.begin(); it != oob_modules.end(); it++)
			if (is_abrev(modname, it->first.c_str())) {
				arg = get_arg_in_braces(arg, temp, STOP_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
				prepare_actionalias(temp, command, sizeof(command) / sizeof(wchar_t) - 1);

				if (is_abrev(command, L"disable")) {
					it->second.submodules.clear();
					command[0] = L'\0';
				} else if (is_abrev(command, L"enable")) {
					if (it->first == L"GMCP") {
						it->second.submodules.insert(L"core");
						it->second.submodules.insert(L"char");
						it->second.submodules.insert(L"room");
						it->second.submodules.insert(L"comm");
						it->second.submodules.insert(L"group");
					} else if (it->first == L"MSDP") {
						it->second.submodules.insert(L"ROOM");
						it->second.submodules.insert(L"AFFECTS");
						it->second.submodules.insert(L"HEALTH");
						it->second.submodules.insert(L"HEALTH_MAX");
						it->second.submodules.insert(L"ACCOUNT_NAME");
						it->second.submodules.insert(L"CHARACTER_NAME");
						it->second.submodules.insert(L"SERVER_ID");
						it->second.submodules.insert(L"SERVER_TIME");
						it->second.submodules.insert(L"SPECIFICATION");
						it->second.submodules.insert(L"AFFECTS");
						it->second.submodules.insert(L"ALIGNMENT");
						it->second.submodules.insert(L"EXPERIENCE");
						it->second.submodules.insert(L"EXPERIENCE_MAX");
						it->second.submodules.insert(L"EXPERIENCE_TNL");
						it->second.submodules.insert(L"EXPERIENCE_TNL_MAX");
						it->second.submodules.insert(L"HEALTH");
						it->second.submodules.insert(L"HEALTH_MAX");
						it->second.submodules.insert(L"LEVEL");
						it->second.submodules.insert(L"MANA");
						it->second.submodules.insert(L"MANA_MAX");
						it->second.submodules.insert(L"MONEY");
						it->second.submodules.insert(L"MOVEMENT");
						it->second.submodules.insert(L"MOVEMENT_MAX");
						it->second.submodules.insert(L"OPPONENT_LEVEL");
						it->second.submodules.insert(L"OPPONENT_HEALTH");
						it->second.submodules.insert(L"OPPONENT_HEALTH_MAX");
						it->second.submodules.insert(L"OPPONENT_NAME");
						it->second.submodules.insert(L"OPPONENT_STRENGTH");
					} else if (it->first == L"MSSP") {
						it->second.submodules.insert(L"NAME");
						it->second.submodules.insert(L"PLAYERS");
						it->second.submodules.insert(L"UPTIME");
						it->second.submodules.insert(L"CRAWL DELAY");
						it->second.submodules.insert(L"HOSTNAME");
						it->second.submodules.insert(L"PORT");
						it->second.submodules.insert(L"CODEBASE");
						it->second.submodules.insert(L"CONTACT");
						it->second.submodules.insert(L"CREATED");
						it->second.submodules.insert(L"ICON");
						it->second.submodules.insert(L"IP");
						it->second.submodules.insert(L"LANGUAGE");
						it->second.submodules.insert(L"LOCATION");
						it->second.submodules.insert(L"MINIMUM AGE");
						it->second.submodules.insert(L"WEBSITE");
						it->second.submodules.insert(L"FAMILY");
						it->second.submodules.insert(L"GENRE");
						it->second.submodules.insert(L"GAMEPLAY");
						it->second.submodules.insert(L"STATUS");
						it->second.submodules.insert(L"GAMESYSTEM");
						it->second.submodules.insert(L"INTERMUD");
						it->second.submodules.insert(L"SUBGENRE");
						it->second.submodules.insert(L"AREAS");
						it->second.submodules.insert(L"HELPFILES");
						it->second.submodules.insert(L"MOBILES");
						it->second.submodules.insert(L"OBJECTS");
						it->second.submodules.insert(L"ROOMS");
						it->second.submodules.insert(L"CLASSES");
						it->second.submodules.insert(L"LEVELS");
						it->second.submodules.insert(L"RACES");
						it->second.submodules.insert(L"SKILLS");
						it->second.submodules.insert(L"ANSI");
						it->second.submodules.insert(L"GMCP");
						it->second.submodules.insert(L"MCCP");
						it->second.submodules.insert(L"MCP");
						it->second.submodules.insert(L"MSDP");
						it->second.submodules.insert(L"MSP");
						it->second.submodules.insert(L"MXP");
						it->second.submodules.insert(L"PUEBLO");
						it->second.submodules.insert(L"UTF-8");
						it->second.submodules.insert(L"VT100");
						it->second.submodules.insert(L"XTERM 256 COLORS");
						it->second.submodules.insert(L"PAY TO PLAY");
						it->second.submodules.insert(L"PAY FOR PERKS");
						it->second.submodules.insert(L"HIRING BUILDERS");
						it->second.submodules.insert(L"HIRING CODERS");
					}

					command[0] = L'\0';
				} else if (is_abrev(command, L"request")) {
					arg = get_arg_in_braces(arg, temp, WITH_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
					prepare_actionalias(temp, command, sizeof(command) / sizeof(wchar_t) - 1);

					vector< wstring > names = split_str(command, L' ');

					for (int i = 0; i < names.size(); i++) {
						if (it->first == L"GMCP") {
							swprintf(temp, L"request \"%ls\"", names[i].c_str());
							send_telnet_subnegotiation(TN_GMCP, temp, wcslen(temp), true);
						} else if (it->first == L"MSDP") {
							swprintf(temp, L"%lcSEND%lc%ls", MSDP_VAR, MSDP_VAL, names[i].c_str());
							send_telnet_subnegotiation(TN_MSDP, temp, wcslen(temp), true);
						} else if (it->first == L"MSSP") {
							send_telnet_command(TN_DO, TN_MSSP);
						}
					}

					command[0] = L'\0';
				} else if (is_abrev(command, L"add")) {
					arg = get_arg_in_braces(arg, temp, WITH_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
					prepare_actionalias(temp, command, sizeof(command) / sizeof(wchar_t) - 1);

					vector< wstring > names = split_str(command, L' ');
					for (int i = 0; i < names.size(); i++)
						it->second.submodules.insert(names[i]);
					
					command[0] = L'\0';
				} else if (is_abrev(command, L"del")) {
					arg = get_arg_in_braces(arg, temp, WITH_SPACES, sizeof(temp) / sizeof(wchar_t) - 1);
					prepare_actionalias(temp, command, sizeof(command) / sizeof(wchar_t) - 1);

					vector< wstring > names = split_str(command, L' ');
					for (int i = 0; i < names.size(); i++) {
						if (it->second.submodules.find(names[i]) != it->second.submodules.end())
							it->second.submodules.erase(names[i]);
					}
					command[0] = L'\0';
				}

				if (!command[0]) {
					swprintf(temp, rs::rs(1300), it->first.c_str());
					set<wstring> submods = it->second.submodules;
					bool first = true;
					for (it2 = submods.begin(); it2 != submods.end(); it2++) {
						if (!first)
							wcscat(temp, L", ");
						wcscat(temp, it2->c_str());
						first = false;
					}
					if (first)
						wcscat(temp, L"-");
					tintin_puts2(temp);
				} else {
					tintin_puts2(rs::rs(1301));
				}

				modfound = true;
				break;
			}
	}
	
	if (!modfound) {
		swprintf(temp, rs::rs(1299));
		bool first = true;
		for (it = oob_modules.begin(); it != oob_modules.end(); it++) {
			if (!first)
				wcscat(temp, L", ");
			wcscat(temp, it->first.c_str());
			first = false;
		}
		if (first)
			wcscat(temp, L"-");
		tintin_puts2(temp);
	}
}

void reset_oob()
{
	check_init();
	if (pRoot)
		delete pRoot;
	pRoot = new COobObject(NULL);
}

void start_gmcp()
{
	set<wstring>::const_iterator it;
	wchar_t buffer[BUFFER_SIZE];
	wstring supports;
	set<wstring> gmcp_modules = oob_modules[L"GMCP"].submodules;
	
	swprintf(buffer, L"core.hello {\"Client\": \"%ls\", \"Version\": \"%ls\"}",
		strProductName, strProductVersion);
	send_telnet_subnegotiation(TN_GMCP, buffer, wcslen(buffer), true);

	for (it = gmcp_modules.begin(); it != gmcp_modules.end(); it++) {
		if (supports.length() > 0)
			supports += L", ";
		supports += L'"';
		supports += (*it);
		supports += L" 1\"";
	}

	swprintf(buffer, L"core.supports.set [%ls]", supports.c_str());
	send_telnet_subnegotiation(TN_GMCP, buffer, wcslen(buffer), true);

	for (it = gmcp_modules.begin(); it != gmcp_modules.end(); it++) {
		swprintf(buffer, L"request %ls", it->c_str());
		send_telnet_subnegotiation(TN_GMCP, buffer, wcslen(buffer), true);
	}
}

void start_msdp()
{
	set<wstring>::const_iterator it;
	wchar_t buffer[BUFFER_SIZE];
	set<wstring> msdp_modules = oob_modules[L"MSDP"].submodules;
	
	swprintf(buffer, L"%lc%ls%lc%ls", 
		MSDP_VAR,
		L"LIST",
		MSDP_VAL,
		L"REPORTABLE_VARIABLES");
	send_telnet_subnegotiation(TN_MSDP, buffer, wcslen(buffer), true);
}

void start_mssp()
{
	/*
	 This function called after successfull handshake (sending TN_DO MSSP), 
	 so there is no need in another request
	 */

	/*
	set<wstring>::const_iterator it;
	wchar_t buffer[BUFFER_SIZE];
	set<wstring> mssp_modules = oob_modules[L"MSSP"].submodules;

	send_telnet_command(TN_DO, TN_MSSP);
	*/
}

int parse_gmcp(const wchar_t *gmcp)
{
	if (pRoot) {
		COobObject *pGMCP = pRoot->child(L"GMCP", true);
		pGMCP->addGMCP(gmcp, true, true);
		
		const wchar_t *end = wcschr(gmcp, L' ');
		if (end) {
			set<wstring> gmcp_modules = oob_modules[L"GMCP"].submodules;
			
			const wchar_t *dot = gmcp, *next_dot;
			while ( (next_dot = wcschr(dot + 1, L'.')) && next_dot < end )
				dot = next_dot;
			if (dot != gmcp)
				end = dot;
			
			wstring module(gmcp, (size_t)(end - gmcp));

			if (gmcp_modules.find(module) != gmcp_modules.end() && mesvar[MSG_MUD_OOB]) {
				wchar_t buff[BUFFER_SIZE];

				swprintf(buff, rs::rs(1298), L"GMCP", module.c_str());
				tintin_puts(buff);
			}
		}
	}
	return 0;
}

int parse_msdp(const wchar_t *msdp, int length)
{
	if (pRoot) {
		wstring gmcp = convert_msdp2gmcp(msdp, length);

		COobObject *pMSDP = pRoot->child(L"MSDP", true);
		pMSDP->addGMCP(gmcp.c_str(), true, true);

		set<wstring> msdp_modules = oob_modules[L"MSDP"].submodules;

		wchar_t *begin = wcschr(msdp, MSDP_VAR);
		wchar_t *end = wcschr(msdp, MSDP_VAL);

		if (begin && end && (begin < end)) {
			wstring module(begin + 1, (size_t)(end - begin - 1));

			if (module == L"REPORTABLE_VARIABLES") {
				COobObject *pVars = pMSDP
					->child(L"reportable", true)
					->child(L"variables", true);

				for (int i = 0; i < pVars->childrenCount(); i++) {
					wstring varname = pVars->child(i, false)->value(false);
					if (msdp_modules.find(varname) != msdp_modules.end()) {
						wchar_t buffer[BUFFER_SIZE];
						swprintf(buffer, L"%lc%ls%lc%ls", 
								MSDP_VAR,
								L"REPORT",
								MSDP_VAL,
								varname.c_str());
						send_telnet_subnegotiation(TN_MSDP, buffer, wcslen(buffer), true);
					}
				}
			}

			if (msdp_modules.find(module) != msdp_modules.end() && mesvar[MSG_MUD_OOB]) {
				wchar_t buff[BUFFER_SIZE];

				swprintf(buff, rs::rs(1298), L"MSDP", module.c_str());
				tintin_puts(buff);
			}
		}
	}
	return 0;
}

wstring convert_msdp2gmcp(const wchar_t *msdp, int len)
{
	wstring ret = L"";
	int prev_tag = 0;
	bool first_entry = true;

	while (len > 0) {
		switch (msdp[0]) {
		case MSDP_VAR:
			switch (prev_tag) {
			case MSDP_ARRAY_CLOSE:
			case MSDP_TABLE_CLOSE:
				ret += L", ";
				break;
			case MSDP_VAR:
			case MSDP_VAL:
				ret += L"\", ";
				break;
			}
			if (!first_entry)
				ret += L'"';
			prev_tag = MSDP_VAR;
			break;
		case MSDP_VAL:
			switch (prev_tag) {
			case MSDP_VAR:
				if (!first_entry)
					ret += L"\":";
				ret += L' ';
				first_entry = false;
				break;
			case MSDP_VAL:
				ret += L"\", ";
				break;
			case MSDP_ARRAY_CLOSE:
			case MSDP_TABLE_CLOSE:
				ret += L", ";
				break;
			}
			switch (msdp[1]) {
			case MSDP_ARRAY_OPEN:
			case MSDP_TABLE_OPEN:
				break;
			default:
				ret += L'"';
				break;
			}
			prev_tag = MSDP_VAL;
			break;
		case MSDP_ARRAY_OPEN:
			ret += L'[';
			prev_tag = MSDP_ARRAY_OPEN;
			break;
		case MSDP_ARRAY_CLOSE:
			if (prev_tag == MSDP_VAL) {
				ret += L'"';
			}
			ret += L']';
			prev_tag = MSDP_ARRAY_CLOSE;
			break;
		case MSDP_TABLE_OPEN:
			ret += L'{';
			prev_tag = MSDP_TABLE_OPEN;
			break;
		case MSDP_TABLE_CLOSE:
			if (prev_tag == MSDP_VAL) {
				ret += L'"';
			}
			ret += L'}';
			prev_tag = MSDP_TABLE_CLOSE;
			break;
		case L'"':
			ret += L"\\\"";
			break;
		case L'\\':
			ret += L"\\\\";
			break;
		case L' ':
		case L'-':
			if (prev_tag == MSDP_VAR)
				ret += L'_';
			else
				ret += msdp[0];
			break;
		default:
			ret += msdp[0];
		}

		msdp++;
		len--;
	}

	switch (prev_tag) {
	case MSDP_VAR:
	case MSDP_VAL:
		ret += L'"';
		break;
	}
	
	return ret;
}

wstring convert_gmcp2msdp(const wchar_t *gmcp, int len)
{
	wstring ret = L"";
	vector<int> containers;
	
	ret += MSDP_VAR;	
	while (len > 0 && gmcp[0] != L' ') {
		ret += gmcp[0];
		gmcp++;
		len--;
	}
	
	bool wait_for_val = true;

	while (len > 0) {
		switch (gmcp[0]) {
		case L'"':
			ret += wait_for_val ? MSDP_VAL : MSDP_VAR;
			for (gmcp++, len--; gmcp[0] != L'"'; gmcp++, len--) {
				if (gmcp[0] == L'\\') {
					gmcp++, len--;
				}
				ret += gmcp[0];
			}
			break;
		case L':':
			wait_for_val = true;
			break;
		case L',':
			if (containers.size() == 0) { 
				//wrong format
			} else {
				wait_for_val = (containers.back() == MSDP_ARRAY_OPEN);
			}
			break;
		case L'[':
			ret += MSDP_VAL;
			ret += MSDP_ARRAY_OPEN;
			containers.push_back(MSDP_ARRAY_OPEN);
			wait_for_val = true;
			break;
		case L']':
			if (containers.size() == 0 || containers.back() != MSDP_ARRAY_OPEN) {
				//wrong format
			} else {
				containers.pop_back();
			}
			ret += MSDP_ARRAY_CLOSE;
			break;
		case L'{':
			ret += MSDP_VAL;
			ret += MSDP_TABLE_OPEN;
			containers.push_back(MSDP_TABLE_OPEN);
			wait_for_val = false;
			break;
		case L'}':
			if (containers.size() == 0 || containers.back() != MSDP_TABLE_OPEN) {
				//wrong format
			} else {
				containers.pop_back();
			}
			ret += MSDP_TABLE_CLOSE;
			break;
		default:
			if (iswdigit(gmcp[0]) || gmcp[0] == L'.') {
				if (wait_for_val) {
					ret += MSDP_VAL;
					wait_for_val = false;
				}
				ret += gmcp[0];
			}
			break;
		}

		gmcp++;
		len--;
	}

	return ret;
}

int parse_mssp(const wchar_t *mssp, int length)
{
	if (pRoot) {
		wstring gmcp = convert_mssp2gmcp(mssp, length);

		COobObject *pMSSP = pRoot->child(L"MSSP", true);
		pMSSP->addGMCP(gmcp.c_str(), false, false);

		set<wstring> mssp_modules = oob_modules[L"MSSP"].submodules;

		wchar_t *begin = wcschr(mssp, MSSP_VAR);
		wchar_t *end = wcschr(mssp, MSSP_VAL);

		if (begin && end && (begin < end)) {
			wstring module(begin + 1, (size_t)(end - begin - 1));

			if (mssp_modules.find(module) != mssp_modules.end() && mesvar[MSG_MUD_OOB]) {
				wchar_t buff[BUFFER_SIZE];

				swprintf(buff, rs::rs(1298), L"MSSP", module.c_str());
				tintin_puts(buff);
			}
		}
	}
	return 0;
}

wstring convert_mssp2gmcp(const wchar_t *mssp, int len)
{
	wstring ret = L"";
	wstring val_buf = L"";
	int prev_tag = 0;
	bool list_value = false;

	ret += L'{';
	while (len > 0) {
		switch (mssp[0]) {
		case MSSP_VAR:
			switch (prev_tag) {
			case MSSP_VAR:
				ret += L": {}, ";
				break;
			case MSSP_VAL:
				ret += L'"';
				ret += val_buf;
				ret += L'"';
				if (list_value)
					ret += L']';
				ret += L", ";
				break;
			}
			val_buf = L"";
			prev_tag = MSSP_VAR;
			break;
		case MSSP_VAL:
			switch (prev_tag) {
			case MSSP_VAR:
				ret += L": ";
				list_value = false;
				break;
			case MSSP_VAL:
				if (!list_value)
					ret += L'[';
				else
					ret += L", ";
				ret += L'"';
				ret += val_buf;
				ret += L'"';
				val_buf = L"";
				list_value = true;
				break;
			}
			prev_tag = MSSP_VAL;
			break;
		case L'"':
			(prev_tag == MSSP_VAR ? ret : val_buf) += L"\\\"";
			break;
		case L'\\':
			(prev_tag == MSSP_VAR ? ret : val_buf) += L"\\\\";
			break;
		case L' ':
		case L'-':
			if (prev_tag == MSSP_VAR)
				ret += L'_';
			else
				val_buf += mssp[0];
			break;
		default:
			(prev_tag == MSSP_VAR ? ret : val_buf) += mssp[0];
		}

		mssp++;
		len--;
	}

	switch (prev_tag) {
	case MSSP_VAR:
		ret += L": {}";
		break;
	case MSSP_VAL:
		ret += L'"';
		ret += val_buf;
		ret += L'"';
		if (list_value)
			ret += L']';
		break;
	}
	ret += L'}';

	return ret;
}

int get_oob_variable(const wchar_t *varname, wchar_t *value, int maxlength)
{
	value[0] = L'\0';
	if (!pRoot || !wcslen(varname))
		return 0;

	COobObject *node = pRoot;
	wstring name = L"";

	for(;;) {
		if ((iswupper(varname[0]) || !varname[0]) && name.length() >0) {
			COobObject *subnode = node->child(name.c_str(), false);

			if (!subnode) { //check for Length, Value1, Value2,... 
				if (!wcscmp(name.c_str(), L"Length")) {
					swprintf(value, L"%d", node->childrenCount());
					return wcslen(value);
				}
				int index;
				if (swscanf(name.c_str(), L"Value%u", &index) == 1) {
					subnode = node->child(index - 1, false);
				} 
				//for dictionaries: $GmcpDictKey1/2/3...
				if (!subnode && swscanf(name.c_str(), L"Key%u", &index) == 1 && index > 0 && index <= node->childrenCount()) {
					wstring key = node->key(index - 1);
					int len = min(maxlength, key.length());
					wcsncpy(value, key.c_str(), len);
					value[len] = L'\0';
					return len;
				}
			}
			if (!subnode)
				return 0;
			node = subnode;

			name = L"";
		}
		if (!varname[0])
			break;
		name += varname[0];
		varname++;
	}

	if (node) {
		wstring val = node->value(false);
		int len = min(maxlength, val.length());
		wcsncpy(value, val.c_str(), len);
		value[len] = L'\0';
		return len;
	}

	return 0;
}
