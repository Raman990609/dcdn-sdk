#include "Config.h"

#define ApiRootUrl "https://pcdn.capell.io"

NS_BEGIN(dcdn)

Config::Config()
{
    mApiRootUrl = ApiRootUrl;
}

int Config::CreateTable(sqlite3* db)
{
    char *errMsg = nullptr;
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS configV1 ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "key TEXT NOT NULL,"
        "val TEXT,"
        "UNIQUE(key));";
    int rc = sqlite3_exec(db, sql, nullptr, 0, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return -1;
    }
    return 1;
}

int Config::loadConfigCallback(void* , int argc, char** argv, char** colName)
{
    const char* key = argv[0];
    const char* val = argv[1];
    if (strcmp(key, "peerId") == 0) {
    } else if (strcmp(key, "token") == 0) {
    }
    return 0;
}

void Config::LoadFromDB(sqlite3* db)
{
    const char* sql = "SELECT key,val FROM configV1;";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, loadConfigCallback, this, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return;
    }
}

int Config::SaveToDB(sqlite3* db)
{
    return 1;
}

NS_END
