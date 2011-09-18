set arg1=%1

IF (%1) == () set arg1="DittoPortable"


Rmdir Ditto /s /q

MkDir Ditto
MkDir Ditto\Help
MkDir Ditto\Language
MkDir Ditto\Themes
MkDir Ditto\Addins

if "%2" == "bit64" goto bit64

copy ..\Release\Ditto.exe Ditto\Ditto.exe
copy ..\Release\focus.dll Ditto\focus.dll
copy ..\Release\Addins\*.dll Ditto\Addins\
copy mfc-crt_10\* Ditto\

if "%2"=="" GOTO skipBit64
if "%2"=="bit32" GOTO skipBit64	
:bit64

copy ..\Release64\Ditto.exe Ditto\Ditto.exe
copy ..\Release64\focus64.dll Ditto\focus64.dll
copy ..\Release64\Addins\*.dll Ditto\Addins\
copy mfc-crt64\* Ditto\

:skipBit64

copy Changes.txt Ditto\Changes.txt

copy Ditto.Settings Ditto\portable

copy ..\Help\*.* Ditto\Help\
copy ..\Debug\language\*.xml Ditto\language\
copy ..\Debug\themes\*.xml Ditto\themes\

pause

7za.exe a -tzip Output\%arg1%.zip "Ditto\*" -r

Rmdir Ditto /s /q


