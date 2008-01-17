Rmdir Ditto /s /q

MkDir Ditto
MkDir Ditto\Help
MkDir Ditto\Language
MkDir Ditto\Themes

copy ..\Release\DittoU.exe Ditto\Ditto.exe
copy ..\Release\sqlite3.dll Ditto\sqlite3.dll
copy ..\Release\AccessToSqlite.dll Ditto\AccessToSqlite.dll
copy ..\Release\focus.dll Ditto\focus.dll
copy ..\zlib\zlib1.dll Ditto\zlib1.dll

copy Changes.txt Ditto\Changes.txt

copy mfc-crt\* Ditto\

copy Ditto.Settings Ditto\Ditto.Settings

copy ..\Help\*.* Ditto\Help\
copy ..\Debug\language\*.xml Ditto\language\
copy ..\Debug\themes\*.xml Ditto\themes\

7za.exe a -tzip Ditto_Portable.zip "Ditto\*" -r

Rmdir Ditto /s /q



