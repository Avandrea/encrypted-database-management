#pragma once

#include <string>
#include "DatabaseException.h"
#include <mutex>
#include <atomic>

class DatabaseManager
{
public:
	typedef std::tuple<	int64_t,			//KEY	(for query only, not for insertion or update)
		int64_t,			// descriptor
		std::string,		// name
		std::string			// surname
	> record_type;

	DatabaseManager(const std::string& path, const std::string& name);
	~DatabaseManager();
	
	bool executeTransaction(const std::string& sqlstatement);
	bool addTable(const std::string& tableName);
	bool addElementToTable(const record_type& newrow, const std::string& tableName);
	 
	bool encrypt(const std::string& pass);

	void startPrintThread(const std::string& tableName);
	void stopPrintThread();
	void startInsertThread(const std::string& tableName);
	void stopInsertThread();

	void closeDatabase();

private:
	std::string m_path;
	std::string m_name;
	void* m_rw_db;
	std::string m_password;

	std::recursive_mutex m_rw_mtx;
	int64_t m_counter = 0;
	void printDbData(const std::string& tableName);
	std::thread m_printdata_thread;
	std::atomic_bool m_printdata_thread_running{ false };
	void insertDbData(const std::string& tableName);
	std::thread m_insertdata_thread;
	std::atomic_bool m_insertdata_thread_running{ false };
};

