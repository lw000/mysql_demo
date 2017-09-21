// SQLServer.cpp : Defines the entry point for the console application.
//

#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#else
#include <uuid/uuid.h>
#endif // WIN32

#include <stdio.h>
#include <iostream>
#include <signal.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <pthread.h>

#include "utils.h"

#include "SQLTable.h"
#include "SQLConnPool.h"

static void* thread_one_action(void *arg);

static void* thread_one_action(void *arg) {
	SQLConnPool::CONNECT conn;
	{
		clock_t t = clock();
		int times = 1;
		for (size_t i = 0; i < times; i++) {
			SQLQuotationTable sqlQuotation(&conn);
			{
				sqlQuotation.reset();
				sqlQuotation.createStatement();
				sqlQuotation.executeQuery("SELECT * FROM quotation;",
						[&sqlQuotation](sql::ResultSet* res)
						{
							int row = res->rowsCount();
							while (res->next())
							{
								QuotationTable quotation;
								quotation.name = res->getString("name").c_str();
								quotation.sale_name = res->getString("sale_name").c_str();
								quotation.quotation_number = res->getString("quotation_number").c_str();
								quotation.create_time = res->getString("create_time").c_str();
								sqlQuotation.vtQuotation.push_back(quotation);
							}
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

	{
		int times = 1;
		clock_t t = clock();

		SQLQuotationTable sqlQuotation(&conn);
		sql::PreparedStatement* pstmt = sqlQuotation.prepareStatement(
				"INSERT INTO quotation VALUES(?,?,?,?);");
		pstmt->setString(1, "");
		pstmt->setString(2, "");
		pstmt->setDateTime(4, "2017-06-21 00:00:00");

		for (int i = 0; i < times; i++) {
#ifdef _WIN32
#define GUID_LEN 64
			char buffer[GUID_LEN] = {0};
			GUID guid;

			if (CoCreateGuid(&guid))
			{
				fprintf(stderr, "create guid error\n");
			}

			_snprintf(buffer, sizeof(buffer),
					"%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X",
					guid.Data1, guid.Data2, guid.Data3,
					guid.Data4[0], guid.Data4[1], guid.Data4[2],
					guid.Data4[3], guid.Data4[4], guid.Data4[5],
					guid.Data4[6], guid.Data4[7]);
			pstmt->setString(3, G2U(buffer));
#undef GUID_LEN
#else
			{
				uuid_t uu;
				int i;
				uuid_generate(uu);

				for (i = 0; i < 16; i++) {
					printf("%02X-", uu[i]);
				}
				printf("\n");
			}

#endif // _WIN32

			sqlQuotation.executeQuery([&sqlQuotation](sql::ResultSet* res)
			{
				int row = res->rowsCount();
			}, [](const std::string & error)
			{
				printf("# error: %s", error.c_str());
			});
		}

		clock_t t1 = clock();
		printf("select [%d] times: %f \n", times,
				((double) t1 - t) / CLOCKS_PER_SEC);

	}
	return NULL;
}

int main(int argc, char** argv) {
//	if (argc < 2)
//		return 0;

	SocketInit sInit;

	SQLConnPool* connPool = SQLConnPool::getInstance();

	do {
//#if 0
//		int r = connPool->createSqlConnPool("localhost", "lw", "qazxsw123", "app_project");
//#else
//		int r = connPool->createConnPool("tcp://localhost:3306", "lw",
//				"qazxsw123", "app_project");
//#endif

#if 0
		int r = connPool->createSqlConnPool("192.168.50.26", "lw", "qazxsw123", "app_project");
#else
		int r = connPool->createConnPool("tcp://192.168.50.26:3306", "lw",
				"qazxsw123", "app_project");
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
				pthread_t thread_one;

				/*std::thread st(thread_one_action);
				 st.join();*/

				int status;
				status = pthread_create(&thread_one, NULL, thread_one_action,
				NULL);
				if (status != 0)
					throw std::runtime_error("Thread creation has failed");

				status = pthread_detach(thread_one);
				if (status != 0)
					throw std::runtime_error("joining thread has failed");
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
