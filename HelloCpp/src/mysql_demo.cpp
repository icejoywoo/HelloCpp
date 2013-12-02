/*
 * mysql_demo.cpp
 *
 *  Created on: 2013年11月30日
 *      Author: icejoywoo
 */

#include <iostream>
#include "mysql_driver.h"
#include "mysql_connection.h"
#include "cppconn/driver.h"
#include "cppconn/statement.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/metadata.h"
#include "cppconn/exception.h"

int main() {
	const char* user = "root";
	const char* passwd = "";
	const char* host = "tcp://localhost:3306";
	const char* database = "test";
	try {
		sql::mysql::MySQL_Driver* driver =
		        sql::mysql::get_mysql_driver_instance();
		sql::Connection* conn = driver->connect(host, user, passwd);
		conn->setSchema(database);
		std::cout << "status: " << conn->isClosed() << std::endl;
		sql::Statement *stmt = conn->createStatement();
		sql::ResultSet *res = stmt->executeQuery("select 1;");
		while (res->next()) {
			std::cout << res->getInt(1) << std::endl;
		}

		res = stmt->executeQuery("select * from pet;");
		while (res->next()) {
			std::cout << res->getString(1) << ",";
			std::cout << res->getString(2) << ",";
			std::cout << res->getString(3) << ",";
			std::cout << res->getInt(4) << ",";
			std::cout << res->getString(5) << ",";
			std::cout << res->getString(6) << std::endl;
		}

		delete res;
		delete stmt;
		delete conn;
	} catch (sql::SQLException& e) {
		std::cout << "# ERR: SQLException in " << __FILE__;
		std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
		std::cout << "# ERR: " << e.what();
		std::cout << " (MySQL error code: " << e.getErrorCode();
		std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
	}
	return 0;
}

