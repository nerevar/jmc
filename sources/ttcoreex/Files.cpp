#include "stdafx.h"
#include "tintin.h"
#include "files.h"
#include <time.h>
#include <io.h>
#include <string>
#include <vector>
#include <map>

#include "Proxy.h"

//vls-begin// base dir
wchar_t DLLEXPORT szBASE_DIR[MAX_PATH];
wchar_t DLLEXPORT szSETTINGS_DIR[MAX_PATH];

struct completenode *complete_head;
void prepare_for_write(const wchar_t *command, const wchar_t *type, const wchar_t *left, const wchar_t *right, const wchar_t *pr, const wchar_t* group, wchar_t *result);
void prepare_for_write(const wchar_t *command, const wchar_t *left, const wchar_t *right, const wchar_t *pr, const wchar_t* group, wchar_t *result);

std::vector<std::wstring> processed_fnames;

//* en
BOOL bSosExact = FALSE;
//*/en

void DLLEXPORT utf16le_to_utf16be(wchar_t *dst, const wchar_t *src, int count)
{
	const unsigned char *psrc = (const unsigned char*)src;
	unsigned char *pdst = (unsigned char*)dst;
	for (int i = 0; i < count*sizeof(wchar_t); i += sizeof(wchar_t)) {
		pdst[i + 1] = *psrc++;
		pdst[i] = *psrc++;
	}
}

// filename -> codepage preservation
static std::map<std::wstring, DWORD> file_codepage;
/*
	Simple codepage detection:
	1) if contents can be decoded as UTF-8 - its UTF-8
	2) if contents has UTF-16 BOM (starts from FEFF or FFFE) - its UTF-16
	3) otherwise its system default ANSI (1251, 1252 etc.)
 */
int DLLEXPORT read_file_contents(const wchar_t *FilePath, wchar_t *Buffer, int Capacity)
{
	FILE *fin = _wfopen(FilePath, L"rb");
	
	if (!fin)
		return 0;
	
	fseek(fin, 0, SEEK_END);
	int size = ftell(fin);

	if (size == 0) {
		fclose(fin);
		if (Buffer)
			Buffer[0] = '\0';
		return 0;
	}

	unsigned char *tmpBuf = new unsigned char[size + 1];
	fseek(fin, 0, SEEK_SET);
	fread(tmpBuf, 1, size, fin);
	tmpBuf[size++] = '\0';
	fclose(fin);

	int req_cap = (Capacity ? Capacity - 1 : 0);

	int ret;
	if ( (ret = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (const char*)tmpBuf, size, Buffer, req_cap)) > 0 ) {
		delete[] tmpBuf;
		file_codepage[std::wstring(FilePath)] = CP_UTF8;
		if (Buffer)
			Buffer[ret] = L'\0';
		return ret + 1;
	}
	if (size >= 2 && tmpBuf[0] == 0xFF && tmpBuf[1] == 0xFE) {
		//if ( (ret = MultiByteToWideChar(1200, MB_ERR_INVALID_CHARS, (const char*)tmpBuf + 2, size - 2, Buffer, Capacity)) > 0 ) {
		{ //1200 stands for UTF-16LE which is available only for managed apps, so just copy-paste contents as if it is UCS-2LE
			if (req_cap > 0) {
				ret = min(req_cap, (size-2) / 2);
				memcpy(Buffer, tmpBuf + 2, ret * 2);
			} else {
				ret = (size - 2) / 2;
			}
			delete[] tmpBuf;
			file_codepage[std::wstring(FilePath)] = 1200;
			if (Buffer)
				Buffer[ret] = L'\0';
			return ret + 1;
		}
	}
	if (size >= 2 && tmpBuf[0] == 0xFE && tmpBuf[1] == 0xFF) {
		//if ( (ret = MultiByteToWideChar(1201, MB_ERR_INVALID_CHARS, (const char*)tmpBuf + 2, size - 2, Buffer, Capacity)) > 0 ) {
		{ //1201 stands for UTF-16BE which is available only for managed apps, so assume UCS-2LE compatibility and swap bytes in each pair
			if (req_cap) {
				ret = min(req_cap, (size-2) / 2);
				utf16le_to_utf16be((wchar_t*)(tmpBuf + 2), Buffer, ret);
			} else {
				ret = (size - 2) / 2;
			}
			delete[] tmpBuf;
			file_codepage[std::wstring(FilePath)] = 1201;
			if (Buffer)
				Buffer[ret] = L'\0';
			return ret + 1;
		}
	}
	if ( (ret = MultiByteToWideChar(CP_ACP, 0, (const char*)tmpBuf, size, Buffer, req_cap)) > 0 ) {
		delete[] tmpBuf;
		file_codepage[std::wstring(FilePath)] = CP_ACP;
		if (Buffer)
			Buffer[ret] = L'\0';
		return ret + 1;
	}
	//something went completely wrong
	delete[] tmpBuf;
	return 0;
}

int DLLEXPORT write_file_contents(const wchar_t *FilePath, const wchar_t *Buffer, int Length)
{
	FILE *fout;
	
	std::map< std::wstring, DWORD >::iterator it = file_codepage.find(std::wstring(FilePath));

	DWORD codepage;
	if (it == file_codepage.end())
		codepage = CP_UTF8;
	else
		codepage = it->second;
	
	fout = _wfopen(FilePath, L"wb");
	if (!fout)
		return -1;

	char *buf;
	int len;

	if (codepage == 1200) {
		fwrite("\xFF\xFE", 1, 2, fout);
		len = Length * 2;
		buf = new char[len];
		memcpy(buf, Buffer, len);
	} else if (codepage == 1201) {
		fwrite("\xFE\xFF", 1, 2, fout);
		len = Length * 2;
		buf = new char[len];
		utf16le_to_utf16be((wchar_t*)buf, Buffer, Length);
	} else {
		len = WideCharToMultiByte(codepage, 0, Buffer, Length, NULL, 0, NULL, NULL);
		buf = new char[len];
		WideCharToMultiByte(codepage, 0, Buffer, Length, buf, len, NULL, NULL);
	}

	int ret = fwrite(buf, 1, len, fout);
	fclose(fout);

	delete[] buf;

	file_codepage[std::wstring(FilePath)] = codepage;

	return ret;
}

/***********************************/
/* read and execute a command file */
/***********************************/
static void process_file(const wchar_t*FilePath, int Size)
{
    wchar_t *buffer, *command;


    EnterCriticalSection(&secReadingConfig);
    ResetEvent(eventReadingHasUse);
    SetEvent(eventReadingConfig);

	buffer = new wchar_t[Size + 1];
	command = new wchar_t[Size + 1];
	read_file_contents(FilePath, buffer, Size);

	buffer[Size] = L'\0';

	wchar_t *src = buffer;
	wchar_t *dst = command;
	wchar_t* cptr = buffer;
	while(Size > 0) {
		wchar_t *start = cptr;

		int nested = 0;
		dst = command;
        for(; *cptr && Size > 0; cptr++, Size--) {
			if (*cptr == DEFAULT_OPEN)
				nested++;
			else if (*cptr == DEFAULT_CLOSE)
				nested--;
			else if (*cptr == L'\r' || *cptr == L'\n') {
				bool skip_newline = false;
				if (nested > 0) {
					skip_newline = true;
				} else {
					wchar_t *nextch = space_out(cptr);
					skip_newline = (*nextch == DEFAULT_OPEN || *nextch == DEFAULT_CLOSE);
				}

				if (!skip_newline) 
					break;
				
				int spaces = space_out(cptr) - cptr - 1;
				cptr += spaces;
				Size -= spaces;
				*(cptr) = L' ';
			}
			*(dst++) = *cptr;
		}

        *cptr=L'\0';
		*(dst++) = L'\0';
		//for(; iswspace(*start); start++); //skip empty lines
		start = space_out(command);
        if ( *start && wcslen(start) ) 
            parse_input(start, TRUE);

		cptr++;
		Size--;
    }
	/*
	wchar_t* cptr = buffer;
    while(Size > 0) {
		wchar_t *start = cptr;
        for(; *cptr && *cptr!=L'\n' && *cptr!=L'\r' && Size > 0; cptr++, Size--);
        *cptr=L'\0';
		for(; iswspace(*start); start++); //skip empty lines
        if ( *start && wcslen(start) ) 
            parse_input(start, TRUE);
		cptr++;
		Size--;
    }
	*/
		
	delete[] command;
	delete[] buffer;

    ResetEvent(eventReadingConfig);
    if (WaitForSingleObject(eventReadingHasUse, 0) == WAIT_OBJECT_0) {
        PostMessage(hwndMAIN, WM_USER+300, 0, 0);
    } else if(WaitForSingleObject(eventReadingFirst, 0) != WAIT_OBJECT_0) {
		//scripts are reloaded on opening profile (OnNewDocument())
		//so we don't need to reload them once again immediately
        //PostMessage(hwndMAIN, WM_USER+300, 0, 0);
    } SetEvent(eventReadingFirst);
    LeaveCriticalSection(&secReadingConfig);
}

void variable_value_filename(wchar_t *arg)
{
	int sz = processed_fnames.size();
	if (sz > 0)
		wcscpy(arg, processed_fnames[sz - 1].c_str());
}

void read_command(wchar_t *arg)
{
    wchar_t message[BUFFER_SIZE];

    wchar_t filename[BUFFER_SIZE], temp[BUFFER_SIZE];
    get_arg_in_braces(arg,temp,WITH_SPACES,sizeof(temp)/sizeof(wchar_t)-1);
	prepare_actionalias(temp, filename,sizeof(filename)/sizeof(wchar_t)-1);

    if ( *filename == 0 ) {
        tintin_puts2(rs::rs(1238));
        return;
    }

    wchar_t fn[MAX_PATH+2];
	int size;

    MakeAbsolutePath(fn, filename, szBASE_DIR);
    if((size = read_file_contents(fn, NULL, 0)) <= 0) {
        swprintf(message,rs::rs(1031), fn);
        tintin_puts2(message);
    } else {
		processed_fnames.push_back(filename);
        process_file(fn, size);
		processed_fnames.pop_back();
    }

    MakeAbsolutePath(fn, L"global.set", szBASE_DIR);
    if((size = read_file_contents(fn, NULL, 0)) <= 0) {
        swprintf(message,rs::rs(1032));
        tintin_puts2(message);
    } else {
		processed_fnames.push_back(L"global.set");
        process_file(fn, size);
		processed_fnames.pop_back();
    }
    
    if (!verbose) {
        tintin_puts2(rs::rs(1033));
        swprintf(message,rs::rs(1034), AliasList.size());
        tintin_puts2(message);
        swprintf(message,rs::rs(1035),ActionList.size());
        tintin_puts2(message);
        swprintf(message,rs::rs(1036),antisubnum);
        tintin_puts2(message);
        swprintf(message,rs::rs(1037),subnum);
        tintin_puts2(message);
        swprintf(message,rs::rs(1038),VarList.size ());
        tintin_puts2(message);
        swprintf(message,rs::rs(1039),HlightList.size());
        tintin_puts2(message);
        swprintf(message,rs::rs(1040),GroupList.size());
        tintin_puts2(message);
    }

    if ( GroupList.find(DEFAULT_GROUP_NAME) == GroupList.end() ) {
        GroupList[DEFAULT_GROUP_NAME] = new CGROUP(DEFAULT_GROUP_NAME);
    }
}

/************************/
/* write a command file */
/************************/
void write_command(wchar_t *arg)
{
	USES_CONVERSION;

	std::wstring set_lines, glob_lines;
	wchar_t set_fn[MAX_PATH+2], glob_fn[MAX_PATH+2];

    wchar_t buffer[BUFFER_SIZE*10], filename[BUFFER_SIZE], group[BUFFER_SIZE];
    struct listnode *nodeptr;
    int i;
    CGROUP* grp ;

    arg=get_arg_in_braces(arg,filename,WITH_SPACES,sizeof(filename)/sizeof(wchar_t)-1);
    arg=get_arg_in_braces(arg,group,STOP_SPACES,sizeof(group)/sizeof(wchar_t)-1);

    if (*filename==L'\0') {
        tintin_puts2(rs::rs(1041));
        return;
    }
    
    if ( *group ) {
        GROUP_INDEX ind = GroupList.find(group);
        if ( ind == GroupList .end() ) {
            wchar_t result[BUFFER_SIZE];
            swprintf ( result, rs::rs(1042), group);
            tintin_puts2(result);
            return;
        }
        grp = ind->second;
    }

    if ( *group == 0 ) {
        MakeAbsolutePath(set_fn, filename, szBASE_DIR);
		set_lines = L"";

        MakeAbsolutePath(glob_fn, L"global.set", szBASE_DIR);
		glob_lines = L"";

        // save messages state
        if ( !mesvar[MSG_ALIAS] ) {
            buffer[0] = cCommandChar ;
            wcscpy(buffer+1, L"message alias OFF\n");
            set_lines += buffer;
        }
        if ( !mesvar[MSG_ACTION] ) {
            buffer[0] = cCommandChar ;
            wcscpy(buffer+1, L"message action OFF\n");
            set_lines += buffer;
        }
        if ( !mesvar[MSG_SUB] ) {
            buffer[0] = cCommandChar ;
            wcscpy(buffer+1, L"message subst OFF\n");
            set_lines += buffer;
        }
        if ( !mesvar[MSG_ANTISUB] ) {
            buffer[0] = cCommandChar ;
            wcscpy(buffer+1, L"message antisub OFF\n");
            set_lines += buffer;
        }
        if ( !mesvar[MSG_HIGH] ) {
            buffer[0] = cCommandChar ;
            wcscpy(buffer+1, L"message high OFF\n");
            set_lines += buffer;
        }
        if ( !mesvar[MSG_VAR] ) {
            buffer[0] = cCommandChar ;
            wcscpy(buffer+1, L"message variable OFF\n");
            set_lines += buffer;
        }
        if ( !mesvar[MSG_GRP] ) {
            buffer[0] = cCommandChar ;
            wcscpy(buffer+1, L"message group OFF\n");
            set_lines += buffer;
        }
        if ( !mesvar[MSG_HOT] ) {
            buffer[0] = cCommandChar ;
            wcscpy(buffer+1, L"message hotkey OFF\n");
            set_lines += buffer;
        }
        if ( !mesvar[MSG_SF] ) {
            buffer[0] = cCommandChar ;
            wcscpy(buffer+1, L"message uses OFF\n");
            set_lines += buffer;
        }
        if ( !mesvar[MSG_LOG] ) {
            buffer[0] = cCommandChar ;
            wcscpy(buffer+1, L"message logs OFF\n");
            set_lines += buffer;
        }
		if ( mesvar[MSG_TELNET] ) {
            buffer[0] = cCommandChar ;
            wcscpy(buffer+1, L"message telnet ON\n");
            set_lines += buffer;
        }
		if ( !mesvar[MSG_MUD_OOB] ) {
            buffer[0] = cCommandChar ;
            wcscpy(buffer+1, L"message oob OFF\n");
            set_lines += buffer;
        }
		if ( !mesvar[MSG_MAPPER] ) {
            buffer[0] = cCommandChar ;
            wcscpy(buffer+1, L"message mapper OFF\n");
            set_lines += buffer;
        }

        // save togglesub/echo/multiaction etc states 
        buffer[0] = cCommandChar ;

        wcscpy(buffer+1, L"multiaction ");
        wcscat (buffer, bMultiAction ? L"on\n" : L"off\n" );
        set_lines += buffer;

        wcscpy(buffer+1, L"multihighlight ");
        wcscat (buffer, bMultiHighlight ? L"on\n" : L"off\n" );
        set_lines += buffer;

        wcscpy(buffer+1, L"presub ");
        wcscat (buffer, presub ? L"on\n" : L"off\n" );
        set_lines += buffer;

        wcscpy(buffer+1, L"echo ");
        wcscat (buffer, echo ? L"on\n" : L"off\n" );
        set_lines += buffer;

        wcscpy(buffer+1, L"ignore ");
        wcscat (buffer, ignore? L"on\n" : L"off\n" );
        set_lines += buffer;

        wcscpy(buffer+1, L"speedwalk ");
        wcscat (buffer, speedwalk ? L"on\n" : L"off\n" );
        set_lines += buffer;

        wcscpy(buffer+1, L"togglesubs ");
        wcscat (buffer, togglesubs ? L"on\n" : L"off\n" );
        set_lines += buffer;

        wcscpy(buffer+1, L"verbat ");
        wcscat (buffer, verbatim ? L"on\n" : L"off\n" );
        set_lines += buffer;

//* en:colon
        wcscpy(buffer+1, L"colon ");
        wcscat (buffer, bColon ? L"replace\n" : L"leave\n" );
        set_lines += buffer;
//* en:comment
        swprintf(buffer, L"%lccomment %lc\n",cCommandChar,cCommentChar);
        set_lines += buffer;
//* en:race
        swprintf(buffer, L"%lcrace format %ls\n",cCommandChar,race_format);
        set_lines += buffer;
//*/en

		//save proxy settings
		if (!ulProxyAddress) {
			prepare_for_write(L"proxy", L"disable", L"", L"", L"", buffer);
		} else {
			wchar_t addr[BUFFER_SIZE];
			if (dwProxyPort)
				swprintf(addr, L"%d.%d.%d.%d:%d", 
					(ulProxyAddress >> 24) & 0xff, 
					(ulProxyAddress >> 16) & 0xff, 
					(ulProxyAddress >>  8) & 0xff, 
					(ulProxyAddress >>  0) & 0xff,
					dwProxyPort);
			else
				swprintf(addr, L"%d.%d.%d.%d", 
					(ulProxyAddress >> 24) & 0xff, 
					(ulProxyAddress >> 16) & 0xff, 
					(ulProxyAddress >>  8) & 0xff, 
					(ulProxyAddress >>  0) & 0xff);
			prepare_for_write(L"proxy", 
				(dwProxyType == PROXY_SOCKS4) ? L"socks4" : L"socks5",
				addr,
				A2W(sProxyUserName),
				A2W(sProxyUserPassword),
				buffer);
		}
		set_lines += buffer;

		//save secure settings
		switch (lTLSType) {
		default:
		case TLS_DISABLED:
			prepare_for_write(L"secure", L"disable", L"", L"", L"", buffer);
			break;
		case TLS_SSL3:
			prepare_for_write(L"secure", L"ssl3", L"ca", 
				(strCAFile.size() ? strCAFile.c_str() : L"clear"), L"", buffer);
			break;
		case TLS_TLS1:
			prepare_for_write(L"secure", L"tls1", L"ca", 
				(strCAFile.size() ? strCAFile.c_str() : L"clear"), L"", buffer);
			break;
		case TLS_TLS1_1:
			prepare_for_write(L"secure", L"tls1.1", L"ca", 
				(strCAFile.size() ? strCAFile.c_str() : L"clear"), L"", buffer);
			break;
		case TLS_TLS1_2:
			prepare_for_write(L"secure", L"tls1.2", L"ca", 
				(strCAFile.size() ? strCAFile.c_str() : L"clear"), L"", buffer);
			break;
		}
		set_lines += buffer;

		//save codepage settings
		prepare_for_write(L"codepage", CPNames[MudCodePage].c_str(), L"", L"", L"", buffer);
		set_lines += buffer;

		//save telnet options
		for(int opt = 0; opt < vEnabledTelnetOptions.size(); opt++) {
			wchar_t optname[64];
			get_telnet_option_name(vEnabledTelnetOptions[opt], optname);
			prepare_for_write(L"telnet", optname, L"on", L"", L"", buffer);
			set_lines += buffer;
		}
		if (bTelnetDebugEnabled)
			prepare_for_write(L"telnet", L"debug", L"on", L"", L"", buffer);
		else
			prepare_for_write(L"telnet", L"debug", L"off", L"", L"", buffer);
		set_lines += buffer;

		//save oob (out-of-band) settings
		map< wstring, oob_module_info >::const_iterator oob_it;
		for (oob_it = oob_modules.begin(); oob_it != oob_modules.end(); oob_it++) {
			set <wstring>::const_iterator oob_sub_it;
			set <wstring> submods = oob_it->second.submodules;
			wstring all_mods = L"";

			for (oob_sub_it = submods.begin(); oob_sub_it != submods.end(); oob_sub_it++) {
				if (all_mods.length() > 0)
					all_mods += L' ';
				all_mods += (*oob_sub_it);
			}

			if (all_mods.length() == 0)
				prepare_for_write(L"oob", oob_it->first.c_str(), L"disable", L"", L"", buffer);
			else
				prepare_for_write(L"oob", oob_it->first.c_str(), L"add", all_mods.c_str(), L"", buffer);

			set_lines += buffer;
		}

		//save broadcast settings
		prepare_for_write(L"broadcast", L"filterip", bBCastFilterIP ? L"on" : L"off", L"", L"", buffer);
		set_lines += buffer;
		
		prepare_for_write(L"broadcast", L"filterport", bBCastFilterPort ? L"on" : L"off", L"", L"", buffer);
		set_lines += buffer;

		wchar_t portnum[BUFFER_SIZE];
		swprintf(portnum, L"%d", wBCastUdpPort);
		prepare_for_write(L"broadcast", L"port", portnum, L"", L"", buffer);
		set_lines += buffer;
		
		prepare_for_write(L"broadcast", bBCastEnabled ? L"enable" : L"disable", L"", L"", L"", buffer);
		set_lines += buffer;
		

		//save end-of-prompt char settings
		if (!bPromptEndEnabled) {
			prepare_for_write(L"promptend", L"disable", L"", L"", L"", buffer);
		} else {
			prepare_for_write(L"promptend", strPromptEndSequence, strPromptEndReplace, L"", L"", buffer);
		}
		set_lines += buffer;


        ALIAS_INDEX ind = AliasList.begin();
        while (ind  != AliasList.end() ) {
            ALIAS* pal = ind->second;
            prepare_for_write(L"alias", ind->first.c_str(), pal->m_strRight.c_str(), 
                L"\0", pal->m_pGroup->m_strName.c_str () , buffer);
			(pal->m_pGroup->m_bGlobal ? glob_lines : set_lines) += buffer;
            ind++;
        }

        ACTION_INDEX aind = ActionList.begin();
        while (aind  != ActionList.end() ) {
            // CActionPtr pac = *aind;
            ACTION* pac = *aind;
            if ( !pac->m_bDeleted ) {
                wchar_t buff[32];
                prepare_for_write(L"action", 
					act_type_to_str((int)pac->m_InputType),
					pac->m_strLeft.c_str(), 
					pac->m_strRight.c_str(), 
                    _itow(pac->m_nPriority, buff, 10) , 
					pac->m_pGroup->m_strName.c_str() , 
					buffer);
                (pac->m_pGroup->m_bGlobal ? glob_lines : set_lines) += buffer;
            }
            aind++;
        }


        VAR_INDEX vind = VarList.begin();
        while ( vind != VarList.end() ) {
            VAR* pvar = vind->second;
            prepare_for_write(L"variable", vind->first.c_str(), pvar->m_strVal.c_str(), pvar->m_bGlobal ? L"global" : L"\0",  L"\0", buffer);
            (pvar->m_bGlobal ? glob_lines : set_lines) += buffer;
            vind++;
        }

        HLIGHT_INDEX hind = HlightList.begin();
        while ( hind != HlightList.end() ) {
            HLIGHT* ph = hind->second;
            prepare_for_write(L"highlight", ph->m_strColor.c_str(), hind->first.c_str(), 
                L"\0", ph->m_pGroup->m_strName.c_str () , buffer);

            (ph->m_pGroup->m_bGlobal ? glob_lines : set_lines) += buffer;
            hind++;
        }
    


      nodeptr=common_antisubs;
      while((nodeptr=nodeptr->next)) {
        prepare_for_write(L"antisubstitute", nodeptr->left,
        nodeptr->right, L"\0",  L"\0",buffer);
        set_lines += buffer;
      } 
  
      nodeptr=common_subs;
      while((nodeptr=nodeptr->next)) {
        prepare_for_write(L"substitute", nodeptr->left, nodeptr->right, L"\0",  L"\0", buffer);
        set_lines += buffer;
      }

      nodeptr=common_pathdirs;
      // !!! TO prevent default patgs writting
      i = 0;
      do {
        nodeptr=nodeptr->next;
        i++;
      } while ( nodeptr && i < 7 );

      while(nodeptr) {
        prepare_for_write(L"pathdir", nodeptr->right, nodeptr->left, L"\0",  L"\0",buffer);
        set_lines += buffer;
        nodeptr=nodeptr->next;
      }

//vls-begin// grouped hotkeys
        // write hotkeys here 
        HOTKEY_INDEX hotind = HotkeyList.begin();
        while ( hotind != HotkeyList.end() ) {
            CHotKey* pHotKey= hotind->second;
            prepare_for_write(L"hot", pHotKey->m_strKey.c_str() ,
                pHotKey->m_strAction.c_str() , L"\0",
                pHotKey->m_pGroup->m_strName.c_str(),  buffer);
            set_lines += buffer;
            hotind++;
        }
//vls-end//

//vls-begin// script files
        SCRIPTFILE_INDEX sfind = ScriptFileList.begin();
        while ( sfind != ScriptFileList.end() ) {
            PCScriptFile psf = *sfind;
            prepare_for_write(L"use", psf->m_strName.c_str(), L"\0", L"\0", L"\0", buffer);
            set_lines += buffer;
            sfind++;
        }
//vls-end//
    
        // write groups states
        GROUP_INDEX gind = GroupList.begin();
        while ( gind != GroupList.end() ) {
            CGROUP* pg = gind->second;
            if ( !pg->m_bEnabled ) {
                swprintf( buffer, L"%cgroup disable %s\n", cCommandChar , pg->m_strName.c_str());
                (pg->m_bGlobal ? glob_lines : set_lines) += buffer;
            } 
            if ( pg->m_bGlobal ) {
                swprintf( buffer, L"%cgroup global %s\n", cCommandChar , pg->m_strName.c_str());
                glob_lines += buffer;
            } else {
                swprintf( buffer, L"%cgroup local %s\n", cCommandChar , pg->m_strName.c_str());
                set_lines += buffer;
            }

            gind++;
        }

//vls-begin// grouped hotkeys
//        // write hotkeys here 
//        HOTKEY_INDEX hotind = HotkeyList.begin();
//        while ( hotind != HotkeyList.end() ) {
//            CHotKey* pHotKey= hotind->second;
//            prepare_for_write("hot", (char*)pHotKey->m_strKey.data() ,
//                (char*)pHotKey->m_strAction.data() , "\0",  "\0",buffer);
//            fputws(buffer, myfile);
//            hotind++;
//        }
//vls-end//

        // write ticksize now 
        swprintf(buffer , L"%cticksize %d\n" , cCommandChar , tick_size );
        set_lines += buffer;
    } else {
        ALIAS_INDEX ind = AliasList.begin();
        while (ind  != AliasList.end() ) {
            ALIAS* pal = ind->second;
            if ( pal->m_pGroup == grp ) {
                prepare_for_write(L"alias", ind->first.c_str(), pal->m_strRight.c_str(), 
                    L"\0", pal->m_pGroup->m_strName.c_str() , buffer);
                set_lines += buffer;
            }
            ind++;
        }

        ACTION_INDEX aind = ActionList.begin();
        while (aind  != ActionList.end() ) {
            // CActionPtr pac = *aind;
            ACTION* pac = *aind;
            wchar_t buff[32];
            if ( pac->m_pGroup == grp && !pac->m_bDeleted ) {
                prepare_for_write(L"action", 
					act_type_to_str((int)pac->m_InputType),
					pac->m_strLeft.data(), 
					pac->m_strRight.data(), 
                    _itow(pac->m_nPriority, buff, 10) , 
					pac->m_pGroup->m_strName.c_str() , 
					buffer);
                set_lines += buffer;
            }
            aind++;
        }


        HLIGHT_INDEX hind = HlightList.begin();
        while ( hind != HlightList.end() ) {
            HLIGHT* ph = hind->second;
            if ( ph->m_pGroup == grp ) {
                prepare_for_write(L"highlight", ph->m_strColor.c_str(), hind->first.c_str(), 
                    L"\0", ph->m_pGroup->m_strName.c_str () , buffer);

                set_lines += buffer;
            }
            hind++;
        }

//vls-begin// grouped hotkeys
        HOTKEY_INDEX hotind = HotkeyList.begin();
        while ( hotind != HotkeyList.end() ) {
            CHotKey* pHotKey= hotind->second;
            prepare_for_write(L"hot", pHotKey->m_strKey.c_str() ,
                pHotKey->m_strAction.c_str() , L"\0",
                pHotKey->m_pGroup->m_strName.c_str(),  buffer);
            set_lines += buffer;
            hotind++;
        }
//vls-end//
    }

	if(set_lines.length() > 0 && write_file_contents(set_fn, set_lines.c_str(), set_lines.length()) <= 0) {
		wchar_t buff[BUFFER_SIZE];
        swprintf(buff,rs::rs(1043), set_fn);
        tintin_puts2(buff);
    }

	if(glob_lines.length() > 0 && write_file_contents(glob_fn, glob_lines.c_str(), glob_lines.length()) <= 0) {
		wchar_t buff[BUFFER_SIZE];
        swprintf(buff,rs::rs(1043), glob_fn);
        tintin_puts2(buff);
    }

    tintin_puts2(rs::rs(1046));
}

void prepare_for_write(const wchar_t *command, const wchar_t *type, const wchar_t *left, const wchar_t *right, const wchar_t *pr, const wchar_t* group, wchar_t *result)
{
  *result=cCommandChar;
  *(result+1)=L'\0';
  wcscat(result, command);
  if (type) {
	  wcscat(result, L" ");
	  wcscat(result, type);
  }
  wcscat(result, L" {");
  wcscat(result, left);
  wcscat(result, L"}");
  if (wcslen(right)!=0) {
    wcscat(result, L" {");
    wcscat(result, right);
    wcscat(result, L"}");
  }
  if (wcslen(pr)!=0) {
    wcscat(result, L" {");
    wcscat(result, pr);
    wcscat(result, L"}");
  }
  if (wcslen(group)!=0) {
    wcscat(result, L" {");
    wcscat(result, group);
    wcscat(result, L"}");
  }
  wcscat(result,L"\n");
}

void prepare_for_write(const wchar_t *command, const wchar_t *left, const wchar_t *right, const wchar_t *pr, const wchar_t* group, wchar_t *result)
{
  prepare_for_write(command, NULL, left, right, pr, group, result);
}

void prepare_quotes(wchar_t *string)
{
  wchar_t s[BUFFER_SIZE], *cpsource, *cpdest;
  int nest=FALSE;
  wcscpy(s, string);

  cpsource=s;
  cpdest=string;

 while(*cpsource) {
    if(*cpsource==L'\\') {
      *cpdest++=*cpsource++;
      if(*cpsource)
        *cpdest++=*cpsource++;
    }
    else if(*cpsource==L'\"' && nest==FALSE) {
      *cpdest++=L'\\';
      *cpdest++=*cpsource++;
    }
    else if(*cpsource==L'{') {
      nest=TRUE;
      *cpdest++=*cpsource++;
    }
    else if(*cpsource==L'}') {
      nest=FALSE;
      *cpdest++=*cpsource++;
    }
    else
      *cpdest++=*cpsource++; 
  }
  *cpdest='\0';
}


//vls-begin// base dir
// Examples:
// base = "c:\mud\jmc";
// 1. abs = "C:\muD\dir\file.ext";
//    loc = "..\dir\file.ext";
// 2. abs = "D:\dir";
//    loc = "D:\dir";
// 3. abs = "C:\mud\jmc\dir1\dir2\file3";
//    loc = "dir1\dir2\file3";

int MakeLocalPath(wchar_t *loc, const wchar_t *abs, const wchar_t *base)
{
    std::wstring sLoc;
    std::wstring sAbs(abs);
    std::wstring sBase(base);
    int iBase = 0;

    int nAbs = sAbs.length();
    int iAbs = 0;
    while ((iAbs = sAbs.find(L'/', iAbs)) != std::wstring::npos) sAbs[iAbs] = L'\\';

    if (sAbs.length() < 4 || sAbs[1] != L':' || sAbs[2] != L'\\') {
        wcscpy(loc, sAbs.c_str());
        return nAbs;
    }

    int nBaseDirs = 0;
    sBase += L'\\';
    for (iBase = 0; iBase < sBase.length(); iBase++) {
        if (sBase[iBase] == L'\\') nBaseDirs++;
    }

    int nMatchDirs = 0;
    int nMin = min(sAbs.length(), sBase.length());
    iAbs = 0;
    for (iBase = 0; iBase < nMin; iBase++) {
        if (towlower(sBase[iBase]) != towlower(sAbs[iBase]))
            break;

        if (sAbs[iBase] == L'\\') {
            iAbs = iBase + 1;
            nMatchDirs++;
        }
    }

    if (nMatchDirs > 0) {
        for (; nMatchDirs < nBaseDirs; nMatchDirs++) {
            sLoc += L"..\\";
        }
    }
    sLoc.append(sAbs, iAbs, sAbs.length() - iAbs);
    if (sLoc.length() >= MAX_PATH) {
        wcsncpy(loc, sLoc.c_str(), MAX_PATH-1);
        loc[MAX_PATH-1] = L'\0';
        return MAX_PATH-1;
    }
    wcscpy(loc, sLoc.c_str());
    return sLoc.length();
}

int MakeAbsolutePath(wchar_t *abs, const wchar_t *loc, const wchar_t *base)
{
    std::wstring sAbs(base);
    std::wstring sLoc(loc);

    int nLoc = sLoc.length();
    int iLoc = 0;
    while ((iLoc = sLoc.find(L'/', iLoc)) != std::wstring::npos) sLoc[iLoc] = L'\\';

    if (sLoc.length() > 3 && sLoc[1] == L':' && sLoc[2] == L'\\') {
        wcscpy(abs, sLoc.c_str());
        return nLoc;
    }

    iLoc = 0;
    int nCurDir = 0;
    while (iLoc < nLoc) {
        if (sLoc[iLoc] == L'.' && nCurDir == 0) {
            if (iLoc + 2 < nLoc && sLoc[iLoc+1] == L'.' && sLoc[iLoc+2] == L'\\') {
                int iAbs = sAbs.rfind(L'\\');
                if (iAbs != std::wstring::npos) {
                    sAbs.resize(iAbs);
                }
                iLoc += 2;
                continue;
            }
        }
        if (sLoc[iLoc] == L'\\') {
            nCurDir = 0;
            iLoc++;
            continue;
        }

        if (nCurDir == 0) {
            sAbs += L'\\';
        }
        sAbs += sLoc[iLoc];
        nCurDir++;
        iLoc++;
    }
    if (sAbs.length() >= MAX_PATH) {
        wcsncpy(abs, sAbs.c_str(), MAX_PATH-1);
        abs[MAX_PATH-1] = L'\0';
        return MAX_PATH-1;
    }
    wcscpy(abs, sAbs.c_str());
    return sAbs.length();
}
//vls-end//


//* en


void sos_command(wchar_t *arg)
{
	wchar_t command[BUFFER_SIZE], paramet[BUFFER_SIZE];

	arg = get_arg_in_braces(arg,command,STOP_SPACES,sizeof(command)/sizeof(wchar_t)-1);
	arg = get_arg_in_braces(arg,paramet,WITH_SPACES,sizeof(paramet)/sizeof(wchar_t)-1);

    int i;
    wchar_t buffer[BUFFER_SIZE];

    if(is_abrev(command,L"mode"))
	{
		if(is_abrev(paramet,L"exact"))
			bSosExact = TRUE;
		else
			bSosExact = FALSE;
		return;
	}

    if(is_abrev(command,L"list"))
	{
		for(i=0;soski[i].name[0];i++)
		{
			swprintf(buffer,L"¹%2d group=%10s;entity=%40s",i,soski[i].group,soski[i].name);
			tintin_puts2(buffer);
		}
	}

    if(is_abrev(command,L"clear"))
	{
		for(i=0;soski[i].name[0];i++)
		{
			soski[i].group[0]=0;
			soski[i].name[0]=0;
		}
		bSosExact = FALSE;
		return;
	}
	
	if(is_abrev(command,L"save"))
	{//save soski

		std::wstring lines = L"";
        wchar_t fn[MAX_PATH+2];
        MakeAbsolutePath(fn, paramet, szBASE_DIR);

		swprintf(buffer,L"%lc%lc%lc JMC Save Our Vars dump\n",cCommandChar,cCommandChar,cCommandChar);
		lines += buffer;
		
		ALIAS_INDEX ind = AliasList.begin();
        while (ind  != AliasList.end() ) 
		{
            ALIAS* pal = ind->second;
			for(i=0;soski[i].name[0];i++)
			{
              if((is_abrev(soski[i].group,L"alias") && (
				  (soski[i].name[0] == '*')||
				 (!bSosExact && is_abrev(soski[i].name,ind->first.c_str()))||
				  (bSosExact && !_wcsicmp(soski[i].name,ind->first.c_str())))))
			  {
                prepare_for_write(L"alias", ind->first.c_str(), pal->m_strRight.c_str(), 
                  L"\0", pal->m_pGroup->m_strName.c_str() , buffer);
                lines += buffer;
			  }
			}
            ind++;
        }

        ACTION_INDEX aind = ActionList.begin();
        while (aind  != ActionList.end() ) 
		{

            ACTION* pac = *aind;
            if ( !pac->m_bDeleted ) 
			{
			  for(i=0;soski[i].name[0];i++)
			  {
              if(is_abrev(soski[i].group,L"action") && (
				  (soski[i].name[0] == L'*')||
				  (!bSosExact && is_abrev(soski[i].name,pac->m_strLeft.c_str()))||
				  (bSosExact && !_wcsicmp(soski[i].name,pac->m_strLeft.c_str()))))
			  {
				  wchar_t buff[32];
				  prepare_for_write(L"action", 
					act_type_to_str((int)pac->m_InputType),
					pac->m_strLeft.c_str(), 
					pac->m_strRight.c_str(), 
                    _itow(pac->m_nPriority, buff, 10) , 
					pac->m_pGroup->m_strName.c_str() , 
					buffer);
                  lines += buffer;
				}
			  }
			}
            aind++;
        }


        VAR_INDEX vind = VarList.begin();
        while ( vind != VarList.end() ) 
		{
            VAR* pvar = vind->second;
			for(i=0;soski[i].name[0];i++)
			{
              if(is_abrev(soski[i].group,L"variable") && (
				  (soski[i].name[0] == L'*')||
				  (!bSosExact && is_abrev(soski[i].name,vind->first.c_str()))||
				  (bSosExact && !_wcsicmp(soski[i].name,vind->first.c_str()))))
			  {
				prepare_for_write(L"variable", vind->first.c_str(), pvar->m_strVal.c_str(), 
					pvar->m_bGlobal ? L"global" : L"\0",  L"\0", buffer);
				lines += buffer;
			  }  
			} 
            vind++;
        }

        HOTKEY_INDEX hotind = HotkeyList.begin();
        while ( hotind != HotkeyList.end() ) {
            CHotKey* pHotKey= hotind->second;
			for(i=0;soski[i].name[0];i++)
			{
            if(is_abrev(soski[i].group,L"hotkey") && (
				(soski[i].name[0] == L'*')||
				(!bSosExact && is_abrev(soski[i].name,pHotKey->m_strKey.c_str()))||
				(bSosExact && !_wcsicmp(soski[i].name,pHotKey->m_strKey.c_str()))))
			{
            prepare_for_write(L"hot", pHotKey->m_strKey.c_str() ,
                pHotKey->m_strAction.c_str() , L"\0",
                pHotKey->m_pGroup->m_strName.c_str(),  buffer);
            lines += buffer;
			}
			}
            hotind++;
        }

		if (lines.length() > 0 && write_file_contents(fn, lines.c_str(), lines.length()) <= 0) {
		    wchar_t buff[BUFFER_SIZE];
            swprintf(buff,rs::rs(1043), fn);
            tintin_puts2(buff);
            return; 
        }
        
		return;
	}
	
	    for(i=0;soski[i].name[0];i++)
			if(is_abrev(soski[i].name,paramet)&&is_abrev(soski[i].group,command))
				return;
		
        for(i=0;soski[i].name[0];i++);
		if(is_abrev(command,L"hotkey") || 
			is_abrev(command,L"alias") || 
			is_abrev(command,L"action") || 
			is_abrev(command,L"variable"))
			wcscpy(buffer,command);
		if(buffer[0] && paramet[0])
            wcscpy(buffer,command);
		else 
			return;
		wcscpy(soski[i].group,buffer);
		wcscpy(soski[i].name,paramet);


	;//new soska
	
}
//*/en
