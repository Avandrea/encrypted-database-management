// databaseManager.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DatabaseManager.h"
#include <windows.h>
#include <iostream>

int main()
{
	std::string db_path = ".";
	std::string db_name;
	std::string table_name = "table1";

	// Insert database name. If it is equal to an existing one the program will open the existing one
	std::cout << "Insert db name: " << std::endl;
	std::cin >> db_name;

	// Insert the password
	std::string pass = "";
	std::cout << "Insert password:" << std::endl;
	std::cin >> pass;
	
	// Choose whether to use an encrypted db or not
	bool toEncrypt = false;
	std::string answer;
	std::cout << "Do you want to encrypt the db? [y/n]" << std::endl;
	std::cin >> answer;
	toEncrypt = answer.compare("n") == 0 ? false : true;

	DatabaseManager db(db_path, db_name);

	if (toEncrypt)
	{
		db.encrypt(pass);
	}	
	else
	{
		db.decrypt(pass);
	}

	db.addTable(table_name);

	// Start thread that prints all the elements of table1
	db.startPrintThread(table_name);
	db.startInsertThread(table_name);

	Sleep(10000);

	db.stopPrintThread();
	db.stopInsertThread();
	//db.closeDatabase();

    return 0;
}

