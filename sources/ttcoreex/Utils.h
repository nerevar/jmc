#include "stdafx.h"
#include "tintin.h"
#include <vector>

string strprintf(char* pszFormat, ...);
string StrPrintfV(char* pszFormat, va_list marker);

std::vector<int> split(const string &s, char delim);