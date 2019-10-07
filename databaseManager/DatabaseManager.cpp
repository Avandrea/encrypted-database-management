#include "stdafx.h"
#include "DatabaseManager.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "SQLiteCpp/VariadicBind.h"
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <cstdlib>

namespace fs = std::experimental::filesystem;
using namespace std;
using namespace DatabaseException;

DatabaseManager::DatabaseManager(const std::string& path, const std::string& name)
{
	m_path = path;
	fs::create_directory(m_path);
	m_path += "/" + name;
	// Create database file if it does not exist.
	if (!fs::exists(m_path))
		std::ofstream{ m_path };
	try
	{
		m_rw_db = new SQLite::Database(m_path, SQLite::OPEN_READWRITE);
	}
	catch (...)
	{
		m_rw_db = NULL;
	}
}

DatabaseManager::~DatabaseManager()
{
	SQLite::Database* engine_db = (SQLite::Database*)m_rw_db;
	engine_db->~Database();
	//delete m_rw_db;

	// Delete db
	//if (fs::exists(m_path)) {
	//	if (remove(m_path.c_str()) != 0)
	//		perror("Error deleting file");
	//	else
	//		puts("File successfully deleted");
	//}
}

// Return the single-quote copy of the input string s.
// This function does NOT check if the input string is already single-quoted.
inline std::string quoteSql(const std::string& s)
{
	return '\'' + s + '\'';
}

bool DatabaseManager::executeTransaction(const string& sqlstatement)
{
	bool rvalue = false;
	if (m_rw_db)
	{
		SQLite::Database* engine_db = (SQLite::Database*)m_rw_db;
		try
		{
			rvalue = engine_db->exec(sqlstatement) != 0;
		}
		catch (const SQLite::Exception& e)
		{
			std::cout << "Cannot execute transaction beacuse of " << e.getErrorStr() << " " << e.getExtendedErrorCode() << std::endl;
			throw Exception(e.getErrorCode(), e.getErrorStr());
		}
		catch (const std::runtime_error& e)
		{
			throw Exception(SQLITE_ERROR, e.what());
		}
	}
	return rvalue;
}

bool DatabaseManager::addTable(const string& tableName) {
	if (m_rw_db) {
		SQLite::Database* engine_db = (SQLite::Database*)m_rw_db;

		// If the db is encrypted set the password and proceed
		if (!engine_db->isUnencrypted(m_path))
			engine_db->key(m_password);

		if (!engine_db->tableExists(tableName)) {
			std::string sqlstatement = "CREATE TABLE " + quoteSql(tableName)
				+ "(descriptor Int, name Text, surname Text);";
			executeTransaction(sqlstatement);
			return true;
		}
	}
	return false;
}

bool DatabaseManager::addElementToTable(const record_type& newrow, const string& tableName) {
	string sqlstatement = "INSERT INTO " +
		tableName +
		" (descriptor, name, surname) VALUES ("
		+ std::to_string(std::get<1>(newrow)) + ","			//ALARMS_TABLE_REF_ID
		+ quoteSql(std::get<2>(newrow)) + ","
		+ quoteSql(std::get<3>(newrow)) + ");";				//RESOURCEUID
	return executeTransaction(sqlstatement);
}

#include <stdio.h>
bool DatabaseManager::encrypt(const std::string& pass) {
	m_password = pass;
	SQLite::Database* engine_db = (SQLite::Database*)m_rw_db;

	auto dbisencrypted = engine_db->isUnencrypted(m_path) ? "false" : "true";
	std::cout << "Database is encrypted? " << dbisencrypted << std::endl;

	if (engine_db->isUnencrypted(m_path)) {
		try
		{
			// Remove encrypted and recreate it
			std::string newpath = "C:\\Private\\Repos\\encrypted-database-management\\databaseManager\\testdb1.db";
			if (remove(newpath.c_str()) != 0)
				perror("Error deleting file");
			else
				puts("File successfully deleted");

			std::ofstream outfile("testdb1.db");

			// Attach new database
			std::string sqlstatement = "ATTACH DATABASE " + quoteSql(newpath) + " AS testdb1 KEY 'testkey';";
			engine_db->exec(sqlstatement);

			// Copy all elements from non-encrypted to encrypted
			sqlstatement = "SELECT sqlcipher_export('testdb1');";
			engine_db->exec(sqlstatement);

			// Detach new database
			sqlstatement = "DETACH DATABASE testdb1;";
			engine_db->exec(sqlstatement);

			// Close old databse
			engine_db->~Database();

			// Open new encrypted database
			m_rw_db = new SQLite::Database(newpath, SQLite::OPEN_READWRITE);
			std::string oldpath = m_path;
			m_path = newpath;

			// Remove non-encrypted database
			if (remove(oldpath.c_str()) != 0)
				perror("Error deleting file");
			else
				puts("File successfully deleted");
		}
		catch (const SQLite::Exception& e)
		{
			std::cout << "Cannot encrypt database beacuse of " << e.getErrorStr() << std::endl;
			return false;
		}
	}
	return true;
}

/* Manage print threads */
void DatabaseManager::printDbData(const std::string& tableName) {
	std::string sqlstatement = "SELECT descriptor, name, surname FROM " + quoteSql(tableName) + ";";

	while (m_printdata_thread_running) {
		// Get data
		if (m_rw_db) {
			SQLite::Database* engine_db = (SQLite::Database*)m_rw_db;

			// If the db is encrypted set the password
			if (!engine_db->isUnencrypted(m_path))
				engine_db->key(m_password);

			try {
				SQLite::Statement query(*engine_db, sqlstatement);
				while (query.executeStep())
				{
					int64_t descriptor = query.getColumn(0).getInt();
					std::string name = query.getColumn(1).getString();
					std::string surname = query.getColumn(2).getString();
					std::cout << "Descriptor: " << descriptor << ", name: " << name << ", surname: " << surname << std::endl;
				}
			}
			catch (const SQLite::Exception& e)
			{
				std::cout << "Cannot insert values beacuse of " << e.getErrorStr() << std::endl;
			}
		}

		std::chrono::milliseconds timespan(1000);
		std::this_thread::sleep_for(timespan);
	}
}

void DatabaseManager::startPrintThread(const std::string& tableName) {
	m_printdata_thread_running = true;
	m_printdata_thread = std::thread(&DatabaseManager::printDbData, this, std::ref(tableName));
}

void DatabaseManager::stopPrintThread() {
	m_printdata_thread_running = false;
	if (m_printdata_thread.joinable())
		m_printdata_thread.join();
}

/* Manage insert threads */
std::string gen_random_string(const int len) {
	srand((unsigned int)time(NULL));
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	char *s = new char[len + 1];
	for (int i = 0; i < len; ++i) {
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	s[len] = 0;

	return s;
}

int gen_random_number() {
	srand((unsigned int)time(NULL));
	return rand();
}

void DatabaseManager::insertDbData(const std::string& tableName) {
	std::string sqlstatement = "SELECT descriptor, name, surname FROM " + quoteSql(tableName) + ";";

	while (m_insertdata_thread_running) {
		// Insert data
		if (m_rw_db) {
			SQLite::Database* engine_db = (SQLite::Database*)m_rw_db;

			// If the db is encrypted set the password
			if (!engine_db->isUnencrypted(m_path))
				engine_db->key(m_password);

			try {
				if (engine_db->tableExists("table1")) {
					std::string sqlstatement = "INSERT INTO " + tableName
						+ " VALUES(" + std::to_string(gen_random_number()) + " ," + quoteSql(gen_random_string(5)) + ", " + quoteSql(gen_random_string(8)) + ");";
					executeTransaction(sqlstatement);
					m_counter++;
				}
			}
			catch (const SQLite::Exception& e) {
				std::cout << "Cannot insert values beacuse of " << e.getErrorStr() << std::endl;
			}
		}

		std::chrono::milliseconds timespan(3000);
		std::this_thread::sleep_for(timespan);
	}
}

void DatabaseManager::startInsertThread(const std::string& tableName) {
	m_insertdata_thread_running = true;
	m_insertdata_thread = std::thread(&DatabaseManager::insertDbData, this, std::ref(tableName));
}

void DatabaseManager::stopInsertThread() {
	m_insertdata_thread_running = false;
	if (m_insertdata_thread.joinable())
		m_insertdata_thread.join();
}

/* Close Database instance */
void DatabaseManager::closeDatabase() {
	SQLite::Database* engine_db = (SQLite::Database*)m_rw_db;
	engine_db->~Database();
}