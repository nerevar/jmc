# Microsoft Developer Studio Project File - Name="ttcoreex" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ttcoreex - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ttcoreex.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ttcoreex.mak" CFG="ttcoreex - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ttcoreex - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ttcoreex - Win32 Release MinDependency" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ttcoreex - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/smc/ttcoreex", KEAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ttcoreex - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\wolfssl" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_TTCORE_DLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib ../recore/debug/recore.lib UUID.lib /nologo /subsystem:windows /dll /map /debug /machine:I386 /out:"../ttcoreex.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "ttcoreex - Win32 Release MinDependency"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseMinDependency"
# PROP BASE Intermediate_Dir "ReleaseMinDependency"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseMinDependency"
# PROP Intermediate_Dir "ReleaseMinDependency"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O1 /D "NDEBUG" /D "_ATL_STATIC_REGISTRY" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_TTCORE_DLL" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib odbc32.lib odbccp32.lib wsock32.lib ../recore/release/recore.lib ad1.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../ttcoreex.dll"
# SUBTRACT LINK32 /debug
# Begin Custom Build - Performing registration
OutDir=.\ReleaseMinDependency
TargetPath=\trash\arda\jmc-clone\jmc\sources\ttcoreex.dll
InputPath=\trash\arda\jmc-clone\jmc\sources\ttcoreex.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ttcoreex - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ttcoreex___Win32_Release"
# PROP BASE Intermediate_Dir "ttcoreex___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O1 /D "NDEBUG" /D "_ATL_STATIC_REGISTRY" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_TTCORE_DLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "..\wolfssl" /D "NDEBUG" /D "_ATL_STATIC_REGISTRY" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_TTCORE_DLL" /FR /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib odbc32.lib odbccp32.lib wsock32.lib ../recore/release/recore.lib ad1.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../ttcoreex.dll"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib wsock32.lib winmm.lib ../recore/release/recore.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../ttcoreex.dll" /libpath:"OpenSSL/lib"
# SUBTRACT LINK32 /debug /nodefaultlib

!ENDIF 

# Begin Target

# Name "ttcoreex - Win32 Debug"
# Name "ttcoreex - Win32 Release MinDependency"
# Name "ttcoreex - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Action.cpp
# End Source File
# Begin Source File

SOURCE=.\Alias.cpp
# End Source File
# Begin Source File

SOURCE=.\Antisub.cpp
# End Source File
# Begin Source File

SOURCE=.\Files.cpp
# End Source File
# Begin Source File

SOURCE=.\Glob.cpp
# End Source File
# Begin Source File

SOURCE=.\group.cpp
# End Source File
# Begin Source File

SOURCE=.\Highligh.cpp
# End Source File
# Begin Source File

SOURCE=.\hotkeys.cpp
# End Source File
# Begin Source File

SOURCE=.\Ivars.cpp
# End Source File
# Begin Source File

SOURCE=.\JmcObj.cpp
# End Source File
# Begin Source File

SOURCE=.\JmcSite.cpp
# End Source File
# Begin Source File

SOURCE=.\Llist.cpp
# End Source File
# Begin Source File

SOURCE=.\Logs.cpp
# End Source File
# Begin Source File

SOURCE=.\Misc.cpp
# End Source File
# Begin Source File

SOURCE=.\nodes.cpp
# End Source File
# Begin Source File

SOURCE=.\Parse.cpp
# End Source File
# Begin Source File

SOURCE=.\Path.cpp
# End Source File
# Begin Source File

SOURCE=.\Proxy.cpp
# End Source File
# Begin Source File

SOURCE=.\ressup.cpp
# End Source File
# Begin Source File

SOURCE=.\Scrfiles.cpp
# End Source File
# Begin Source File

SOURCE=.\Secure.cpp
# End Source File
# Begin Source File

SOURCE=.\Session.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Substitu.cpp
# End Source File
# Begin Source File

SOURCE=.\System.cpp
# End Source File
# Begin Source File

SOURCE=.\telnet.cpp
# End Source File
# Begin Source File

SOURCE=.\Text.cpp
# End Source File
# Begin Source File

SOURCE=.\Ticks.cpp
# End Source File
# Begin Source File

SOURCE=.\ttcoreex.cpp
# End Source File
# Begin Source File

SOURCE=.\ttcoreex.def
# End Source File
# Begin Source File

SOURCE=.\ttcoreex.idl
# ADD MTL /tlb ".\ttcoreex.tlb" /h "ttcoreex.h" /iid "ttcoreex_i.c" /Oicf
# End Source File
# Begin Source File

SOURCE=.\ttcoreex.rc
# End Source File
# Begin Source File

SOURCE=.\Utils.cpp
# End Source File
# Begin Source File

SOURCE=.\Variable.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cmds.h
# End Source File
# Begin Source File

SOURCE=.\files.h
# End Source File
# Begin Source File

SOURCE=.\JmcObj.h
# End Source File
# Begin Source File

SOURCE=.\JmcSite.h
# End Source File
# Begin Source File

SOURCE=.\Logs.h
# End Source File
# Begin Source File

SOURCE=.\Proxy.H
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\ressup.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\telnet.H
# End Source File
# Begin Source File

SOURCE=.\TINTIN.H
# End Source File
# Begin Source File

SOURCE=.\TINTINX.H
# End Source File
# Begin Source File

SOURCE=.\ttcoreex.h
# End Source File
# Begin Source File

SOURCE=.\ttcoreexCP.h
# End Source File
# Begin Source File

SOURCE=.\ttobjects.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\JmcObj.rgs
# End Source File
# Begin Source File

SOURCE=.\JmcSite.rgs
# End Source File
# End Group
# Begin Source File

SOURCE=.\ttcoreex.tlb
# End Source File
# End Target
# End Project
