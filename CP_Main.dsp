# Microsoft Developer Studio Project File - Name="CP_Main" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=CP_MAIN - WIN32 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CP_Main.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CP_Main.mak" CFG="CP_MAIN - WIN32 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CP_Main - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "CP_Main - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/CopyPro/CP_Main", CAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CP_Main - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "EncryptDecrypt\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "AFTER_98" /FAs /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ws2_32.lib riched20.lib kernel32.lib user32.lib gdi32.lib winspool.lib Winmm.lib EncryptDecrypt.lib Version.Lib /nologo /subsystem:windows /map /debug /machine:I386 /out:"Release/Ditto.exe" /libpath:"focusdll\Release"

!ELSEIF  "$(CFG)" == "CP_Main - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "EncryptDecrypt\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "AFTER_98" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib riched20.lib kernel32.lib user32.lib gdi32.lib winspool.lib Winmm.lib EncryptDecryptD.lib Version.Lib Pdh.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/Ditto.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "CP_Main - Win32 Release"
# Name "CP_Main - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AlphaBlend.cpp
# End Source File
# Begin Source File

SOURCE=.\AlphaBlend.h
# End Source File
# Begin Source File

SOURCE=.\ArrayEx.h
# End Source File
# Begin Source File

SOURCE=.\ComboBoxSearch.cpp
# End Source File
# Begin Source File

SOURCE=.\ComboBoxSearch.h
# End Source File
# Begin Source File

SOURCE=.\DialogResizer.cpp
# End Source File
# Begin Source File

SOURCE=.\FormattedTextDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\FormattedTextDraw.h
# End Source File
# Begin Source File

SOURCE=.\GroupStatic.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupStatic.h
# End Source File
# Begin Source File

SOURCE=.\HyperLink.cpp
# End Source File
# Begin Source File

SOURCE=.\memdc.h
# End Source File
# Begin Source File

SOURCE=.\NumberEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\NumberEdit.h
# End Source File
# Begin Source File

SOURCE=.\QListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\QListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\RichEditCtrlEx.cpp
# End Source File
# Begin Source File

SOURCE=.\RichEditCtrlEx.h
# End Source File
# Begin Source File

SOURCE=.\ToolTipEx.cpp
# End Source File
# Begin Source File

SOURCE=.\ToolTipEx.h
# End Source File
# Begin Source File

SOURCE=.\WndEx.cpp
# End Source File
# Begin Source File

SOURCE=.\WndEx.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\About.cpp
# End Source File
# Begin Source File

SOURCE=.\AddType.cpp
# End Source File
# Begin Source File

SOURCE=.\BitmapHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\Client.cpp
# End Source File
# Begin Source File

SOURCE=.\CopyProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\CP_Main.cpp
# End Source File
# Begin Source File

SOURCE=.\CP_Main.rc
# End Source File
# Begin Source File

SOURCE=.\DatabaseUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\DataTable.cpp
# End Source File
# Begin Source File

SOURCE=.\FormatSQL.cpp
# End Source File
# Begin Source File

SOURCE=.\FriendDetails.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupCombo.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupName.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupTree.cpp
# End Source File
# Begin Source File

SOURCE=.\InternetUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MainTable.cpp
# End Source File
# Begin Source File

SOURCE=.\Misc.cpp
# End Source File
# Begin Source File

SOURCE=.\MoveToGroupDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionFriends.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsGeneral.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsKeyBoard.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsQuickPaste.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsStats.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\ProcessCopy.cpp
# End Source File
# Begin Source File

SOURCE=.\ProcessPaste.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\QPasteWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\QuickPaste.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectDB.cpp
# End Source File
# Begin Source File

SOURCE=.\Server.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\SystemTray.cpp
# End Source File
# Begin Source File

SOURCE=.\TypesTable.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\About.h
# End Source File
# Begin Source File

SOURCE=.\AddType.h
# End Source File
# Begin Source File

SOURCE=.\BitmapHelper.h
# End Source File
# Begin Source File

SOURCE=.\Client.h
# End Source File
# Begin Source File

SOURCE=.\CopyProperties.h
# End Source File
# Begin Source File

SOURCE=.\CP_Main.h
# End Source File
# Begin Source File

SOURCE=.\DatabaseUtilities.h
# End Source File
# Begin Source File

SOURCE=.\DataTable.h
# End Source File
# Begin Source File

SOURCE=.\DialogResizer.h
# End Source File
# Begin Source File

SOURCE=.\FormatSQL.h
# End Source File
# Begin Source File

SOURCE=.\FriendDetails.h
# End Source File
# Begin Source File

SOURCE=.\GroupCombo.h
# End Source File
# Begin Source File

SOURCE=.\GroupName.h
# End Source File
# Begin Source File

SOURCE=.\GroupTree.h
# End Source File
# Begin Source File

SOURCE=.\HyperLink.h
# End Source File
# Begin Source File

SOURCE=.\IEncryption.h
# End Source File
# Begin Source File

SOURCE=.\InternetUpdate.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MainTable.h
# End Source File
# Begin Source File

SOURCE=.\Misc.h
# End Source File
# Begin Source File

SOURCE=.\MoveToGroupDlg.h
# End Source File
# Begin Source File

SOURCE=.\OptionFriends.h
# End Source File
# Begin Source File

SOURCE=.\OptionsGeneral.h
# End Source File
# Begin Source File

SOURCE=.\OptionsKeyBoard.h
# End Source File
# Begin Source File

SOURCE=.\OptionsQuickPaste.h
# End Source File
# Begin Source File

SOURCE=.\OptionsSheet.h
# End Source File
# Begin Source File

SOURCE=.\OptionsStats.h
# End Source File
# Begin Source File

SOURCE=.\OptionsTypes.h
# End Source File
# Begin Source File

SOURCE=.\OptionsUtilities.h
# End Source File
# Begin Source File

SOURCE=.\ProcessCopy.h
# End Source File
# Begin Source File

SOURCE=.\ProcessPaste.h
# End Source File
# Begin Source File

SOURCE=.\ProgressWnd.h
# End Source File
# Begin Source File

SOURCE=.\QPasteWnd.h
# End Source File
# Begin Source File

SOURCE=.\QuickPaste.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SelectDB.h
# End Source File
# Begin Source File

SOURCE=.\Server.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SystemTray.h
# End Source File
# Begin Source File

SOURCE=.\TypesTable.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\back.bmp
# End Source File
# Begin Source File

SOURCE=.\res\back1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\back2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bmp00003.bmp
# End Source File
# Begin Source File

SOURCE=.\res\closed_f.bmp
# End Source File
# Begin Source File

SOURCE=.\res\CP_Main.ico
# End Source File
# Begin Source File

SOURCE=.\res\CP_Main.rc2
# End Source File
# Begin Source File

SOURCE=.\res\CP_MainDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Ditto.ico
# End Source File
# Begin Source File

SOURCE=.\res\Ditto_NoCopyCb.ico
# End Source File
# Begin Source File

SOURCE=.\res\HorizontalScrollBarLeftArrow.bmp
# End Source File
# Begin Source File

SOURCE=.\res\HorizontalScrollBarRightArrow.bmp
# End Source File
# Begin Source File

SOURCE=.\res\HorizontalScrollBarSpan.bmp
# End Source File
# Begin Source File

SOURCE=.\res\HorizontalScrollBarThumb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\open_fol.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\VerticleScrollbarBottom.bmp
# End Source File
# Begin Source File

SOURCE=.\res\VerticleScrollBarDownArrow.bmp
# End Source File
# Begin Source File

SOURCE=.\res\VerticleScrollBarSpan.bmp
# End Source File
# Begin Source File

SOURCE=.\res\VerticleScrollBarThumb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\VerticleScrollbarTop.bmp
# End Source File
# Begin Source File

SOURCE=.\res\VerticleScrollBarUpArrow.bmp
# End Source File
# End Group
# Begin Group "Exclude From Build"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Debug\focus.dll
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Release\focus.dll
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\EncryptDecryptD.lib
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\EncryptDecrypt.lib
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\focusdll\Release\focus.lib
# End Source File
# End Target
# End Project
