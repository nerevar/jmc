#include "stdafx.h"
#include "tintin.h"
#include <string>

const string
	HTML_TAG = "i",
	TAG_OPEN = "<i>",
	TAG_CLOSE = "</i>",
	BR = "\n",
	CSS_LIGHT_COLOR = "light";

string html_header(
	"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
	"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
	"<head>\n"
	"    <meta http-equiv=\"content-type\" content=\"text/html; charset=windows-1251\" />\n"
	"    <link rel=\"stylesheet\" type=\"text/css\" href=\"fixedsys.css\">\n"
	"    <style type=\"text/css\">\n"
	"        body {color:#ccc;background-color: #000;margin:0;padding:0;}\n"
	"        pre {font: 16px 'FixedsysExcelsior3.01Regular', 'Courier New', Monospace;color: #ccc;}\n"
	"        i {font-style: normal;color: #ccc;}\n"
	"        i.l {color: #fff;}\n"
	"        .bl{color:#555} .l.bl{color:#888} .b_bl{background-color:#555}\n"
	"        .r {color:#c00} .l.r {color:#f00} .b_r {background-color:#c00}\n"
	"        .g {color:#0c0} .l.g {color:#0f0} .b_g {background-color:#0c0}\n"
	"        .y {color:#cc0} .l.y {color:#ff0} .b_y {background-color:#cc0}\n"
	"        .b {color:#00c} .l.b {color:#00f} .b_b {background-color:#00c}\n"
	"        .m {color:#c0c} .l.m {color:#f0f} .b_m {background-color:#c0c}\n"
	"        .c {color:#0cc} .l.c {color:#0ff} .b_c {background-color:#0cc}\n"
	"        .w {color:#ccc} .l.w {color:#fff} .b_w {background-color:#ccc}\n" 
	" </style>\n"
	"</head>\n"
	"<body><pre>\n"
);

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