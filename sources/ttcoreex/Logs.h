#include "stdafx.h"
#include "tintin.h"
#include <string>

using namespace std;

string TAG_OPEN = "<i>";
string TAG_CLOSE = "</i>";
string BR = "\r\n";

string html_header(
	"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\r\n"
	"<html xmlns=\"http://www.w3.org/1999/xhtml\">\r\n"
	"<head>\r\n"
	"	<meta http-equiv=\"content-type\" content=\"text/html; charset=windows-1251\" />\r\n"
	"	<style type=\"text/css\">\r\n"
	"		body {color:#ccc;background-color: #000;margin:0;padding:0;}\r\n"
	"		pre {font: 16px 'FixedsysExcelsior3.01Regular', 'Courier New', Monospace;color: #ccc;}\r\n"
	"		i {font-style: normal;}\r\n"
	"		.bb	{color:#555} .b	{color:#888}\r\n"
	"		.dr	{color:#c00} .lr {color:#f00}\r\n"
	"		.dg {color:#0c0} .lg {color:#0f0}\r\n"
	"		.dy {color:#cc0} .ly {color:#ff0}\r\n"
	"		.db {color:#00c} .lb {color:#00f}\r\n"
	"		.dm {color:#c0c} .lm {color:#f0f}\r\n"
	"		.dc {color:#0cc} .lc {color:#0ff}\r\n"
	"		.dw {color:#ccc} .lw {color:#fff}\r\n"	
	"	</style>\r\n"
	"</head>\r\n"
	"<body><pre>\r\n"
);

string html_footer(
	"\r\n</pre></body></html>"
);

static const char* dtable[] = 
{
	"b",	
	"dr",	
	"dg",
	"dy",	
	"db",	
	"dm",	
	"dc",	
	"dw",
};

static const char* ltable[] = 
{
	"b",
	"lr",
	"lg",
	"ly",
	"lb",
	"lm",
	"lc",
	"lw"
};
