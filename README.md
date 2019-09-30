# encrypted-database-management

This program manages an encrypted database using SQLiteCpp and sqlicipher.

## Requirements
* [SQLiteCpp v2.4.0](https://github.com/SRombauts/SQLiteCpp)
* [sqlcipher v3.4.2](https://github.com/sqlcipher/sqlcipher)
* [OpenSSL v1.0.2t](https://slproweb.com/products/Win32OpenSSL.html)
* [TCL v8.5.18](https://www.activestate.com/products/activetcl/downloads/)

## Compiling

**OpenSSL**  
Download version 1.0.2t from the link and install. The file to be downloaded is:  
![image](https://github.com/Avandrea/encrypted-database-management/blob/master/images/openssl_installer.png)

Come si fa l'encryption del db? Ci sono già librerie che lo fanno?  
Si, sqlcipher. Si compila come sqlite. https://github.com/sqlcipher/sqlcipher (guida generica per modificare makefile per windows https://github.com/sqlitebrowser/sqlitebrowser/wiki/Win64-setup-%E2%80%94-Compiling-SQLCipher) 

API sqlcipher https://www.zetetic.net/sqlcipher/sqlcipher-api/ 

Download openssl x64 https://slproweb.com/products/Win32OpenSSL.html 

Download tcl https://www.activestate.com/products/activetcl/downloads/ (bisogna fare l' account gratuito). In alternativa si può anche compilare ma è complesso. Aggiungere la bin al path delle variabili d'ambiente 

Apri la Developer Command Prompt di visual studio 2015 e lancia call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64  per inizializzare le variabili. 

Cd dentro alla cartella del progetto mkdir build e cd build.  

Copia dentro alla cartella build il Makefile modificato linkato alla fine di questa lista. 

Lancia all'interno della cartella build I seguenti comandi  
nmake /f Makefile.msc TOP=..\ 
 
nmake /f Makefile.msc sqlite3.c TOP=..\ 
nmake /f Makefile.msc sqlite3.dll TOP=..\ 
  
nmake /f Makefile.msc sqlite3.exe TOP=..\ 
 
nmake /f Makefile.msc test TOP=..\ 

Quando finisce copia sqlite.c, sqlite.h e shell.c dentro alla cartella sqlite3 del progetto sqlitecpp e ricompila sqlitecpp 

Infine aggiungi al progetto finale le include di sqlitecpp ("C:\Libs\SQLiteCpp-master\include") le directory per le lib (C:\Libs\SQLiteCpp-master\build\x64\Release) e le lib (sqlite3.lib;SQLiteCpp.lib;"C:\Libs\OpenSSL-Win64\lib\libeay32.lib") 

<<Makefile.msc>>
SQLITECPP 

https://github.com/SRombauts/SQLiteCpp 

Funzioni di sqlitecpp http://fossil.twicetwo.com/index.pl/epic-quest-land/artifact/41105b83e76bdd5e 

Soluzione problema lib http://thebugfreeblog.blogspot.com/2012/08/compiling-sqlcipher-for-windows.html 

 
