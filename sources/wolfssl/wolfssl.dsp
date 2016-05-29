# Microsoft Developer Studio Project File - Name="wolfssl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=wolfssl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wolfssl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wolfssl.mak" CFG="wolfssl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wolfssl - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "wolfssl - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/smc/wolfssl", VDAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wolfssl - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WOLFSSL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /w /W0 /GX /O2 /I ".\\" /D "WOLFSSL_LIB" /D "BUILDING_WOLFSSL" /D "WOLFSSL_DLL" /D "WOLFSSL_USER_SETTINGS" /D "CYASSL_USER_SETTINGS" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WOLFSSL_EXPORTS" /D "HAVE_MEMMOVE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../wolfssl.dll"

!ELSEIF  "$(CFG)" == "wolfssl - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WOLFSSL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".\\" /D "WOLFSSL_LIB" /D "BUILDING_WOLFSSL" /D "WOLFSSL_DLL" /D "WOLFSSL_USER_SETTINGS" /D "CYASSL_USER_SETTINGS" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "WOLFSSL_EXPORTS" /D "HAVE_MEMOVE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../wolfssl.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "wolfssl - Win32 Release"
# Name "wolfssl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\wolfcrypt\src\aes.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\arc4.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\asn.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\blake2b.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\camellia.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\chacha.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\chacha20_poly1305.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\coding.c
# End Source File
# Begin Source File

SOURCE=.\src\crl.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\des3.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\dh.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\dsa.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\ecc.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\error.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\hash.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\hc128.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\hmac.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\integer.c
# End Source File
# Begin Source File

SOURCE=.\src\internal.c
# End Source File
# Begin Source File

SOURCE=.\src\io.c
# End Source File
# Begin Source File

SOURCE=.\src\keys.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\logging.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\md4.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\md5.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\memory.c
# End Source File
# Begin Source File

SOURCE=.\src\ocsp.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\pkcs7.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\poly1305.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\pwdbased.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\rabbit.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\random.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\ripemd.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\rsa.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\sha.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\sha256.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\sha512.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\signature.c
# End Source File
# Begin Source File

SOURCE=.\src\ssl.c
# End Source File
# Begin Source File

SOURCE=.\src\tls.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\wc_encrypt.c
# End Source File
# Begin Source File

SOURCE=.\wolfcrypt\src\wc_port.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\user_settings.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
