/* Autoconf patching by David Hedbor, neotron@lysator.liu.se */
/*********************************************************************/
/* file: utils.c - some utility-functions                            */
/*                             TINTIN III                            */
/*          (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t             */
/*                     coded by peter unold 1992                     */
/*********************************************************************/
#include "stdafx.h"
#include "tintin.h"
#include <vector>
#include <string>
#include <sstream>

using namespace std;

void syserr(wchar_t* msg);

int is_all_digits(const wchar_t *number)
{
	int i, ok = 1;
	if (!number[0])
		return 0;
    for (i = 0; number[i]; i++) {
        if (!iswdigit(number[i])) {
            ok = 0;
            break;
        }
    }
	return ok;
}

/*********************************************/
/* return: TRUE if s1 is an abrevation of s2 */
/*********************************************/
int is_abrev(const wchar_t *s1, const wchar_t *s2)
{
//vls-begin// bugfix
//    return(s1[0]==s2[0] && !strncmp(s2, s1, strlen(s1)));
    return(towlower(s1[0])==towlower(s2[0]) && !wcsnicmp(s2, s1, wcslen(s1)));
//vls-end//
}


/*************************************************/
/* print system call error message and terminate */
/*************************************************/
void syserr(wchar_t* msg)
{
  extern int errno;
  wchar_t ErrMsg[256];

  swprintf(ErrMsg,rs::rs(1211),msg, errno);
  ShowError(ErrMsg);
  // EndApplication();
}

wstring StrPrintfV(wchar_t* pszFormat, va_list marker)
{
	/*
	string data;
	data.reserve(256);
	while( _vsnprintf(&data[0], data.capacity(), pszFormat, marker) == -1 )
           data.reserve(data.capacity() + 256);
    return data;
	*/
	
    vector<wchar_t> data;
        data.reserve(256);
    while( _vsnwprintf(data.begin(), data.capacity(), pszFormat, marker) == -1 )
           data.reserve(data.capacity() + 256);
    return data.begin();
	
}

wstring strprintf(wchar_t* pszFormat, ...)
{
    va_list marker;
    va_start(marker, pszFormat);
    wstring str = StrPrintfV(pszFormat, marker);
    va_end(marker);
    return str;
}


std::vector<int> split(const wstring &s, wchar_t delim) {
    vector<int> elems;
    wstringstream ss(s);
    wstring item;

    while(std::getline(ss, item, delim) && (item.length() > 0)) {
         elems.push_back(_wtoi(item.c_str()));
    }
    return elems;
}

std::vector< wstring > split_str(const wstring &s, wchar_t delim) {
	vector< wstring > elems;
    wstringstream ss(s);
    wstring item;

    while(std::getline(ss, item, delim) && (item.length() > 0)) {
         elems.push_back(item);
    }
    return elems;
}
