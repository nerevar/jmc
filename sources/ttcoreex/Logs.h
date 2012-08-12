#include "stdafx.h"
#include "tintin.h"
#include <string>

const string
	HTML_TAG = "i",
	TAG_OPEN = "<i>",
	TAG_CLOSE = "</i>",
	BR = "\n",
	CSS_LIGHT_COLOR = "l";

string html_footer(
	"\n</pre></body></html>"
);

static const string css_colors[] = 
{
	"bl",	
	"r",	
	"g",
	"y",	
	"b",	
	"m",	
	"c",	
	"w",
};

static const string css_bg_colors[] = 
{
	"b_bl",	
	"b_r",	
	"b_g",
	"b_y",	
	"b_b",	
	"b_m",	
	"b_c",	
	"b_w",
};
