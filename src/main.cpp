// SQLServer.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <iostream>
#include <signal.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <pthread.h>

#include "utils.h"
#include "net.h"

#include "SQLTable.h"
#include "SQLConnPool.h"

static void* thread_select_action(void *arg);
static void* thread_insert_action(void *arg);

static void* thread_select_action(void *arg) {
	SQLConnPool::CONNECT conn;
	{
		clock_t t = clock();
		int times = 10;
		for (size_t i = 0; i < times; i++) {
			SQLQuotationTable sqlQuotation(&conn);
			{
				sqlQuotation.reset();
				sqlQuotation.createStatement();
				sqlQuotation.executeQuery("SELECT * FROM quotation;",
						[&sqlQuotation](sql::ResultSet* res)
						{
							int row = res->rowsCount();
//							while (res->next())
//							{
//								QuotationTable quotation;
//								quotation.name = res->getString("name").c_str();
//								quotation.sale_name = res->getString("sale_name").c_str();
//								quotation.quotation_number = res->getString("quotation_number").c_str();
//								quotation.create_time = res->getString("create_time").c_str();
//								sqlQuotation.vtQuotation.push_back(quotation);
//							}
						}, [](const std::string & error)
						{
							printf("# error: %s", error.c_str());
						});
			}
		}
		clock_t t1 = clock();
		printf("select [%d] times: %f \n", times,
				((double) t1 - t) / CLOCKS_PER_SEC);
	}

	return NULL;
}

static void* thread_insert_action(void *arg) {
	SQLConnPool::CONNECT conn;
	{
		int times = 10;

		clock_t t = clock();

		SQLQuotationTable sqlQuotation(&conn);
		sql::PreparedStatement* pstmt = sqlQuotation.prepareStatement(
				"INSERT INTO quotation VALUES(?,?,?,?);");

		for (int i = 0; i < times; i++) {
			pstmt->setString(1, "new bunisses");
			pstmt->setString(2, "liwei");
 			std::string uuid = make_uuidstring();
 			pstmt->setString(3, uuid.c_str());
 			time_t t = time(NULL);
 			struct tm* tt = localtime(&t);
 			char tm_buff[64];
 			sprintf(tm_buff, "%d-%d-%d %d:%d:%d", tt->tm_year, tt->tm_wday,
 					tt->tm_yday, tt->tm_hour, tt->tm_min, tt->tm_sec);

			pstmt->setDateTime(4, "2017-06-21 00:00:00");

			sqlQuotation.executeQuery([&sqlQuotation](sql::ResultSet* res)
			{
				int row = res->rowsCount();
			}, [](const std::string & error)
			{
				printf("# error: %s", error.c_str());
			});
		}

		clock_t t1 = clock();
		printf("insert [%d] times: %f \n", times,
				((double) t1 - t) / CLOCKS_PER_SEC);

	}

	return 0;
}

int main(int argc, char** argv) {
//	if (argc < 2)
//		return 0;

	SocketInit sInit;

	SQLConnPool* connPool = SQLConnPool::getInstance();

	do {
#ifdef _WIN32
	#if 0
			int r = connPool->createSqlConnPool("localhost", "lw", "qazxsw123", "app_project");
	#else
			int r = connPool->createConnPool("tcp://localhost:3306", "lw",
					"qazxsw123", "app_project");
	#endif

#else
#if 0
		int r = connPool->createSqlConnPool("192.168.50.26", "lw", "qazxsw123", "app_project");
#else
		int r = connPool->createConnPool("tcp://192.168.50.26:3306", "lw",
			"qazxsw123", "app_project");
#endif
#endif



		if (r != 0)
			break;

		{
			sql::Connection* conn = connPool->getConnection();
			sql::PreparedStatement* stmt = conn->prepareStatement(
					"SELECT * FROM config WHERE work_day=?;");
			stmt->setInt(1, 22);
			sql::ResultSet* res = stmt->executeQuery();
			int row = res->rowsCount();
			ConfigTable config;
			while (res->next()) {
				config.proportion = res->getDouble(1);
				config.meal_supplement = res->getDouble(2);
				config.overtime = res->getDouble(3);
				config.finance = res->getDouble(4);
				config.work_day = res->getInt(5);
				config.print();
			}
			connPool->recycleConnection(conn);
		}

		{
			sql::Connection* conn = connPool->getConnection();
			SQLUserTable user(conn);
			{
				user.reset();
				user.createStatement();
				user.executeQuery("SELECT * FROM user;", [](sql::ResultSet* res)
				{
					UserTable user;
					while (res->next()) {
						user.name = res->getString("name");
						user.sex = res->getInt("sex");
						user.position = res->getInt("position");
						user.wages = res->getDouble("wages");
						user.average_wages = res->getDouble("average_wages");
						user.department = res->getInt("department");
						user.print();
					}
				}, [](const std::string & error)
				{

				});
			}

			{
				user.reset();
				user.createStatement();
				user.executeQuery("SELECT * FROM user WHERE id=1;",
						[](sql::ResultSet* res)
						{
							UserTable user;
							while (res->next()) {
								user.name = res->getString(1);
								user.sex = res->getInt(2);
								user.position = res->getInt(3);
								user.wages = res->getDouble(4);
								user.average_wages = res->getDouble(5);
								user.department = res->getInt(6);
								user.print();
							}
						}, [](const std::string & error)
						{
							printf("# error: %s", error.c_str());
						});
			}

			{
				user.reset();
				sql::PreparedStatement* pstmt = user.prepareStatement(
						"SELECT * FROM user WHERE id=?;");
				pstmt->setInt(1, 2);
				user.executeQuery([](sql::ResultSet* res)
				{
					UserTable user;
					while (res->next()) {
						user.name = res->getString(1);
						user.sex = res->getInt(2);
						user.position = res->getInt(3);
						user.wages = res->getDouble(4);
						user.average_wages = res->getDouble(5);
						user.department = res->getInt(6);
						user.print();
					}
				}, [](const std::string & error)
				{
					printf("# error: %s", error.c_str());
				});
			}

			{
				user.reset();
				sql::PreparedStatement* pstmt = user.prepareStatement(
						"UPDATE user  SET sex = ? WHERE name = ?;");
				pstmt->setInt(1, 1);
				pstmt->setString(2, ("李伟"));
				user.executeQuery([](sql::ResultSet* res)
				{
					UserTable user;
					while (res->next()) {
						user.name = res->getString(1);
						user.sex = res->getInt(2);
						user.position = res->getInt(3);
						user.wages = res->getDouble(4);
						user.average_wages = res->getDouble(5);
						user.department = res->getInt(6);
						user.print();
					}
				}, [](const std::string & error)
				{
					printf("# error: %s", error.c_str());
				});
			}

			connPool->recycleConnection(conn);
		}

		{
			for (int i = 0; i < 1; i++) {
				/*std::thread st(thread_one_action);
				 st.join();*/

				{
					pthread_t thread_one;
					int status;
					status = pthread_create(&thread_one, NULL,
							thread_select_action,
							NULL);
					if (status != 0)
						throw std::runtime_error("Thread creation has failed");

					status = pthread_detach(thread_one);
					if (status != 0) {
						throw std::runtime_error("joining thread has failed");
					}
				}

				{
					pthread_t thread_one;
					int status;
					status = pthread_create(&thread_one, NULL,
							thread_insert_action,
							NULL);
					if (status != 0)
						throw std::runtime_error("Thread creation has failed");

					status = pthread_detach(thread_one);
					if (status != 0) {
						throw std::runtime_error("joining thread has failed");
					}
				}

			}

			//pthread_t thread_one;
			//struct st_worker_thread_param *param = new st_worker_thread_param;
			//param->conn = SQLConnPool::getInstance()->getGetConnection();

			///*std::thread st(thread_one_action, (void *)param);
			//st.join();*/

			//int status;
			//status = pthread_create(&thread_one, NULL, thread_one_action, (void *)param);
			//if (status != 0)
			//	throw std::runtime_error("Thread creation has failed");

			//status = pthread_join(thread_one, NULL);
			//if (status != 0)
			//	throw std::runtime_error("joining thread has failed");
			//delete param;
		}
	} while (0);

	int c = getchar();

	return 0;
}
