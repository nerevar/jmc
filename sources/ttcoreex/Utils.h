#include "stdafx.h"
#include "tintin.h"
#include <vector>

wstring strprintf(wchar_t* pszFormat, ...);
wstring StrPrintfV(wchar_t* pszFormat, va_list marker);

std::vector<int> split(const wstring &s, wchar_t delim);