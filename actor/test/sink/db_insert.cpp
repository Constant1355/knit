#include <vector>
#include <iostream>
#include "../../sink/dump/sqlite3.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int main()
{
    sqlite3 *db = nullptr;
    int rc = sqlite3_open("test.db", &db);
    if (rc)
    {
        std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        exit(0);
    }
    else
    {
        std::cout << "Opened database successfully" << std::endl;
    }

    std::string name = "wanzhi";
    int tick = 1234321;

    std::vector<uint8_t> data{1, 2, 3, 4, 5};
    int length = data.size();

    std::string sql = "INSERT INTO SENSORS_SPI_MESSAGE (NAME, TICK, LENGTH, DATA) values(?,?,?,?)";

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), sql.length(), &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        std::cout << "sql error: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    for (size_t idx = 0; idx < 1000; ++idx)
    {
        sqlite3_bind_text(stmt, 1, name.c_str(), name.length(), SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, idx);
        sqlite3_bind_int(stmt, 3, length);
        sqlite3_bind_blob(stmt, 4, data.data(), data.size(), NULL);
        int res = sqlite3_step(stmt);
        sqlite3_reset(stmt);

        if (res == SQLITE_DONE)
        {
            std::cout << "Insert success." << std::endl;
        }
        else
        {
            std::cout << "Insert failed. Errorcode = " << res << std::endl;
        }
    }

    sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);

    sqlite3_finalize(stmt);

    sqlite3_close(db);

    return 0;
}