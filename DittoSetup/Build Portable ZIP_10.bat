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

copy mfc-crt\vcruntime140.dll Ditto\vcruntime140.dll
copy mfc-crt\vccorlib140.dll Ditto\vccorlib140.dll
copy mfc-crt\msvcp140.dll Ditto\msvcp140.dll
copy mfc-crt\mfcm140u.dll Ditto\mfcm140u.dll
copy mfc-crt\mfc140u.dll Ditto\mfc140u.dll
copy mfc-crt\mfc140enu.dll Ditto\mfc140enu.dll

copy ..\Release\icuuc58.dll; Ditto\icuuc58.dll
copy ..\Release\icuin58.dll; Ditto\icuin58.dll
copy ..\Release\icutu58.dll; Ditto\icutu58.dll
copy ..\Release\icuio58.dll; Ditto\icuio58.dll
copy ..\Release\icudt58.dll; Ditto\icudt58.dll

if "%2"=="" GOTO skipBit64
if "%2"=="bit32" GOTO skipBit64	
:bit64

copy ..\Release64\Ditto.exe Ditto\Ditto.exe
copy ..\Release64\Addins\*.dll Ditto\Addins\

copy mfc-crt64\vcruntime140.dll Ditto\vcruntime140.dll
copy mfc-crt64\vccorlib140.dll Ditto\vccorlib140.dll
copy mfc-crt64\msvcp140.dll Ditto\msvcp140.dll
copy mfc-crt64\mfcm140u.dll Ditto\mfcm140u.dll
copy mfc-crt64\mfc140u.dll Ditto\mfc140u.dll
copy mfc-crt64\mfc140enu.dll Ditto\mfc140enu.dll

copy ..\Release64\icuuc58.dll; Ditto\icuuc58.dll
copy ..\Release64\icuin58.dll; Ditto\icuin58.dll
copy ..\Release64\icutu58.dll; Ditto\icutu58.dll
copy ..\Release64\icuio58.dll; Ditto\icuio58.dll
copy ..\Release64\icudt58.dll; Ditto\icudt58.dll

:skipBit64

copy Changes.txt Ditto\Changes.txt

copy portable Ditto\portable

copy ..\Help\*.* Ditto\Help\
copy ..\Debug\language\*.xml Ditto\language\
copy ..\Debug\themes\*.xml Ditto\themes\

pause

7za.exe a -tzip Output\%arg1%.zip "Ditto\*" -r

Rmdir Ditto /s /q


