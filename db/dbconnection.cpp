#include <iostream>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>

void test_db_connection()
{
    try {
        sql::Driver* driver = get_driver_instance();
        std::unique_ptr<sql::Connection> con(
            driver->connect("tcp://127.0.0.1:3306", "root", "")
        );
        con->setSchema("erms");
        std::cout << "Connection to database succeeded!" << std::endl;
    }
    catch (sql::SQLException& e) {
        std::cout << "Connection failed!" << std::endl;
        std::cout << "# ERR: " << e.what() << std::endl;
        std::cout << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cout << "SQLState: " << e.getSQLState() << std::endl;
    }
}