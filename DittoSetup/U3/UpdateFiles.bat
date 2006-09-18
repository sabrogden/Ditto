del Device\Help\* /q
del Device\language\* /q

copy ..\..\Release\DittoU.exe Device\Ditto.exe
copy ..\..\Release\sqlite3.dll Device\sqlite3.dll
copy ..\..\Help\*.htm Device\Help\
copy ..\..\Debug\language\*.xml Device\language\

del Ditto.u3p

7za.exe a -tzip Ditto.u3p "device\*.*" -r -x!Entries.*
7za.exe a -tzip Ditto.u3p "host\*.*" -r -x!Entries.*
7za.exe a -tzip Ditto.u3p "manifest\*.*" -r -x!Entries.*

u3p2exe.exe


