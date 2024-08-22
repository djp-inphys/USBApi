# Microsoft Developer Studio Project File - Name="VtAPI_lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=VtAPI_lib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "VtAPI_lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "VtAPI_lib.mak" CFG="VtAPI_lib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "VtAPI_lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "VtAPI_lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "VtAPI_lib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Zp8 /MD /W3 /GR /GX /O2 /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "VT_STATIC" /D "_AFXDLL" /D "SENSOR" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x412 /d "NDEBUG"
# ADD RSC /l 0x412 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "VtAPI_lib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Zp8 /MDd /W3 /Gm /GR /GX /ZI /Od /D "_DEBUG" /D "SENSOR" /D "WIN32" /D "_MBCS" /D "_LIB" /D "VT_STATIC" /D "_AFXDLL" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x412 /d "_DEBUG"
# ADD RSC /l 0x412 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "VtAPI_lib - Win32 Release"
# Name "VtAPI_lib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\VtErrors.cpp
# End Source File
# Begin Source File

SOURCE=.\VtImage.cpp
# End Source File
# Begin Source File

SOURCE=.\VtSys.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="C:\Program Files\Microsoft Visual Studio\VC98\Include\BASETSD.H"
# End Source File
# Begin Source File

SOURCE=..\ez_lib\ezusb_lib.h
# End Source File
# Begin Source File

SOURCE=..\include\status_strings.h
# End Source File
# Begin Source File

SOURCE=..\include\utils.h
# End Source File
# Begin Source File

SOURCE=.\VtABDiff.h
# End Source File
# Begin Source File

SOURCE=.\VtAPI.h
# End Source File
# Begin Source File

SOURCE=.\VtDataset.h
# End Source File
# Begin Source File

SOURCE=.\VtErrors.h
# End Source File
# Begin Source File

SOURCE=.\VthdsAPI.h
# End Source File
# Begin Source File

SOURCE=.\VthdsCalib.h
# End Source File
# Begin Source File

SOURCE=.\VthdsImpAPI.h
# End Source File
# Begin Source File

SOURCE=.\VthdsLineParser.h
# End Source File
# Begin Source File

SOURCE=.\VtImage.h
# End Source File
# Begin Source File

SOURCE=.\VtPanoramicCalibration.h
# End Source File
# Begin Source File

SOURCE=.\VtParser.h
# End Source File
# Begin Source File

SOURCE=.\VtpcAPI.h
# End Source File
# Begin Source File

SOURCE=.\VtpcImpAPI.h
# End Source File
# Begin Source File

SOURCE=.\VtpcLineParser.h
# End Source File
# Begin Source File

SOURCE=.\VtPipeData.h
# End Source File
# Begin Source File

SOURCE=.\VtSys.h
# End Source File
# Begin Source File

SOURCE=.\VtSysdefs.h
# End Source File
# Begin Source File

SOURCE=..\ez_lib\VtUsbDriver.h
# End Source File
# Begin Source File

SOURCE=..\include\wd_ver.h
# End Source File
# Begin Source File

SOURCE=..\include\wdu_lib.h
# End Source File
# Begin Source File

SOURCE=..\include\windrvr.h
# End Source File
# End Group
# End Target
# End Project
