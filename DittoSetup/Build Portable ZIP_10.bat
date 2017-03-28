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
copy ..\Release\Addins\*.dll Ditto\Addins\
copy mfc-crt_10\*.dll Ditto\
copy ..\Release\icuuc55.dll; Ditto\icuuc55.dll
copy ..\Release\icuin55.dll; Ditto\icuin55.dll
copy ..\Release\icutu55.dll; Ditto\icutu55.dll
copy ..\Release\icuio55.dll; Ditto\icuio55.dll
copy ..\Release\icudt55.dll; Ditto\icudt55.dll

if "%2"=="" GOTO skipBit64
if "%2"=="bit32" GOTO skipBit64	
:bit64

copy ..\Release64\Ditto.exe Ditto\Ditto.exe
copy ..\Release64\Addins\*.dll Ditto\Addins\
copy mfc-crt64\*.dll Ditto\
copy ..\Release64\icuuc55.dll; Ditto\icuuc55.dll
copy ..\Release64\icuin55.dll; Ditto\icuin55.dll
copy ..\Release64\icutu55.dll; Ditto\icutu55.dll
copy ..\Release64\icuio55.dll; Ditto\icuio55.dll
copy ..\Release64\icudt55.dll; Ditto\icudt55.dll

:skipBit64

copy Changes.txt Ditto\Changes.txt

copy portable Ditto\portable

copy ..\Help\*.* Ditto\Help\
copy ..\Debug\language\*.xml Ditto\language\
copy ..\Debug\themes\*.xml Ditto\themes\

pause

7za.exe a -tzip Output\%arg1%.zip "Ditto\*" -r

Rmdir Ditto /s /q


