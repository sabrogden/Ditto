set arg1=%1

IF (%1) == () set arg1="DittoPortable"

cd DittoSetup


Rmdir Ditto /s /q

MkDir Ditto
MkDir Ditto\Help
MkDir Ditto\Language
MkDir Ditto\Themes
MkDir Ditto\Addins

if "%2" == "bit64" goto bit64

copy ..\Release\Ditto.exe Ditto\Ditto.exe
copy ..\Release\ICU_Loader.dll Ditto\ICU_Loader.dll
copy ..\Release\Addins\*.dll Ditto\Addins\

copy "%WINDIR%\SysWOW64\vcruntime140.dll" Ditto\vcruntime140.dll
copy "%WINDIR%\SysWOW64\msvcp140.dll" Ditto\msvcp140.dll
copy "%WINDIR%\SysWOW64\mfc140u.dll" Ditto\mfc140u.dll

if "%2"=="" GOTO skipBit64
if "%2"=="bit32" GOTO skipBit64	
:bit64

copy ..\Release64\Ditto.exe Ditto\Ditto.exe
copy ..\Release64\ICU_Loader.dll Ditto\ICU_Loader.dll
copy ..\Release64\Addins\*.dll Ditto\Addins\

copy "%WINDIR%\System32\vcruntime140.dll" Ditto\vcruntime140.dll
copy "%WINDIR%\System32\vcruntime140_1.dll" Ditto\vcruntime140_1.dll
copy "%WINDIR%\System32\msvcp140.dll" Ditto\msvcp140.dll
copy "%WINDIR%\System32\mfc140u.dll" Ditto\mfc140u.dll

:skipBit64

copy Changes.txt Ditto\Changes.txt

copy portable Ditto\portable

copy ..\Debug\language\*.xml Ditto\language\
copy ..\Debug\themes\*.xml Ditto\themes\

7z a -tzip Output\%arg1%.zip "Ditto\*" -r

Rmdir Ditto /s /q

cd ..
