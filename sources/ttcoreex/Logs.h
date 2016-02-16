#include "stdafx.h"
#include "tintin.h"
#include <string>

//decrease fps to 25Hz max.
const int MIN_HTML_FRAMES_DELAY_MS = 40;

const string
	HTML_TAG = "i",
	TAG_OPEN = "<i>",
	TAG_CLOSE = "</i>",
	BR = "\n",
	CSS_LIGHT_COLOR = "l";

string html_footer(
	"\n</pre></div></body></html>"
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
