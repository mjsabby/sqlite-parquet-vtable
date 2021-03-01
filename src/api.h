#include <cstdint>

#ifdef _WIN32
#define APIDLLEXPORT __declspec(dllexport)
#else
#define APIDLLEXPORT
#endif

extern "C" void ParquetSQLiteProcessWideInitExported(char *tmp);
extern "C" int ParquetSQLiteOpenDatabaseExported(const char *path, void **retVal);
extern "C" int ParquetSQLiteCloseDatabaseExported(void *db);
extern "C" int ParquetSQLiteExecExported(void *db, const char *sql, int (*callback)(void *, int, char **, char **), void *cbdata,
                                         char **errmsg);
extern "C" void ParquetSQLiteFreeMemoryExported(void *str);