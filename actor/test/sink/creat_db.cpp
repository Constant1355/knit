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

    /* Create SQL statement */
    std::string sql = "CREATE TABLE SENSORS_SPI_MESSAGE("
                      "ID INTEGER  PRIMARY KEY     NOT NULL,"
                      "NAME           TEXT    NOT NULL,"
                      "TICK           INTEGER      NOT NULL,"
                      "LENGTH         INTEGER ,"
                      "DATA           BLOB );";

    /* Execute SQL statement */
    char *zErrMsg = 0;
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        fprintf(stdout, "Table created successfully\n");
    }

    sqlite3_close(db);

    return 0;
}