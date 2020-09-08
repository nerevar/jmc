#include "stdafx.h"
#include "tintin.h"
#include <string>

//decrease fps to 25Hz max.
const int MIN_HTML_FRAMES_DELAY_MS = 40;

const wchar_t
	HTML_TAG[] = L"i",
	TAG_OPEN[] = L"<i>",
	TAG_CLOSE[] = L"</i>",
	BR[] = L"\n",
	CSS_LIGHT_COLOR[] = L"l";

const wchar_t html_footer[] =
	L"\n</pre></div></body></html>";

const wchar_t css_colors[][3] = 
{
	L"bl",	
	L"r",	
	L"g",
	L"y",	
	L"b",	
	L"m",	
	L"c",	
	L"w",
};

const wchar_t css_bg_colors[][5] = 
{
	L"b_bl",	
	L"b_r",	
	L"b_g",
	L"b_y",	
	L"b_b",	
	L"b_m",	
	L"b_c",	
	L"b_w",
};
