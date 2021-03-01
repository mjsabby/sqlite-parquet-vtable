#include "api.h"

extern "C" void APIDLLEXPORT ParquetSQLiteProcessWideInit(char *tmp)
{
    ParquetSQLiteProcessWideInitExported(tmp);
}

extern "C" APIDLLEXPORT int ParquetSQLiteOpenDatabase(const char *path, void **retVal)
{
    return ParquetSQLiteOpenDatabaseExported(path, retVal);
}

extern "C" APIDLLEXPORT int ParquetSQLiteCloseDatabase(void *db)
{
    return ParquetSQLiteCloseDatabaseExported(db);
}

extern "C" APIDLLEXPORT int ParquetSQLiteExec(void *db, const char *sql, int (*callback)(void *, int, char **, char **), void *cbdata,
                                              char **errmsg)
{
    return ParquetSQLiteExecExported(db, sql, callback, cbdata, errmsg);
}

extern "C" APIDLLEXPORT void ParquetSQLiteFreeMemory(void *str)
{
    ParquetSQLiteFreeMemoryExported(str);
}