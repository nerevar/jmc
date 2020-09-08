# Microsoft Developer Studio Project File - Name="recore" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=recore - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "recore.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "recore.mak" CFG="recore - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "recore - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "recore - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/smc/recore", VDAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "recore - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RECORE_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /w /W0 /GX /O1 /I ".\\" /D "SUPPORT_PCRE16" /D "HAVE_CONFIG_H" /D "PCRE_STATIC" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../recore.dll"

!ELSEIF  "$(CFG)" == "recore - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RECORE_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\\" /D "HAVE_CONFIG_H" /D "PCRE_STATIC" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../recore.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "recore - Win32 Release"
# Name "recore - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\PCRE.DEF
# End Source File
# Begin Source File

SOURCE=.\pcre16_byte_order.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_chartables.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_compile.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_config.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_dfa_exec.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_exec.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_fullinfo.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_get.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_globals.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_jit_compile.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_maketables.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_newline.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_ord2utf16.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_refcount.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_string_utils.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_study.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_tables.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_ucd.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_utf16_utils.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_valid_utf16.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_version.c
# End Source File
# Begin Source File

SOURCE=.\pcre16_xclass.c
# End Source File
# Begin Source File

SOURCE=.\pcre_byte_order.c
# End Source File
# Begin Source File

SOURCE=.\pcre_chartables.c
# End Source File
# Begin Source File

SOURCE=.\pcre_compile.c
# End Source File
# Begin Source File

SOURCE=.\pcre_config.c
# End Source File
# Begin Source File

SOURCE=.\pcre_dfa_exec.c
# End Source File
# Begin Source File

SOURCE=.\pcre_exec.c
# End Source File
# Begin Source File

SOURCE=.\pcre_fullinfo.c
# End Source File
# Begin Source File

SOURCE=.\pcre_get.c
# End Source File
# Begin Source File

SOURCE=.\pcre_globals.c
# End Source File
# Begin Source File

SOURCE=.\pcre_jit_compile.c
# End Source File
# Begin Source File

SOURCE=.\pcre_maketables.c
# End Source File
# Begin Source File

SOURCE=.\pcre_newline.c
# End Source File
# Begin Source File

SOURCE=.\pcre_ord2utf8.c
# End Source File
# Begin Source File

SOURCE=.\pcre_refcount.c
# End Source File
# Begin Source File

SOURCE=.\pcre_string_utils.c
# End Source File
# Begin Source File

SOURCE=.\pcre_study.c
# End Source File
# Begin Source File

SOURCE=.\pcre_tables.c
# End Source File
# Begin Source File

SOURCE=.\pcre_ucd.c
# End Source File
# Begin Source File

SOURCE=.\pcre_valid_utf8.c
# End Source File
# Begin Source File

SOURCE=.\pcre_version.c
# End Source File
# Begin Source File

SOURCE=.\pcre_xclass.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Config.h
# End Source File
# Begin Source File

SOURCE=.\Pcre.h
# End Source File
# Begin Source File

SOURCE=.\pcre_internal.h
# End Source File
# Begin Source File

SOURCE=.\pcre_scanner.h
# End Source File
# Begin Source File

SOURCE=.\pcrecpp.h
# End Source File
# Begin Source File

SOURCE=.\pcrecpp_internal.h
# End Source File
# Begin Source File

SOURCE=.\pcreposix.h
# End Source File
# Begin Source File

SOURCE=.\ucp.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
