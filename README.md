# sqlite-parquet-vtable

A SQLite [virtual table](https://sqlite.org/vtab.html) extension to expose Parquet files as SQL tables. You may also find [csv2parquet](https://github.com/cldellow/csv2parquet/) useful.

This [blog post](https://cldellow.com/2018/06/22/sqlite-parquet-vtable.html) provides some context on why you might use this.

## Installing

### Building on Linux

```
./build.sh
```

This will generate `libParquetSQLite.so` and `libParquetSQLite.dbg` in the root folder where you ran `build.sh` from.

### Building on Windows

```
build.bat
```

This will generate `ParquetSQLite.dll` and `ParquetSQLite.pdb` in the root folder where you ran `build.bat` from.

## Using

The module that is generated exposes the following platform ABI entry points:

* `ParquetSQLiteProcessWideInit`
* `ParquetSQLiteOpenDatabase`
* `ParquetSQLiteExec`
* `ParquetSQLiteFreeMemory`
* `ParquetSQLiteCloseDatabase`

A typical workflow is as follows:

* ParquetSQLiteProcessWideInit("\tmpDir")
* ParquetSQLiteOpenDatabase("\tmpDir\tmp.db", void** outParamSQLitePointer)
* ParquetSQLiteExec(outParamSQLitePointer, "CREATE VIRTUAL TABLE MyDemo FROM parquet('c:/test1.parquet', 'c:/test2.parquet')", PerResultCallBackFunc, ContextPointer, char** outParamErrorMessage)
* ParquetSQLiteExec(outParamSQLitePointer, "SELECT COUNT(*) FROM MyDemo WHERE SomeColumnInParquetFile = 5", ...)
* More invocations of `ParquetSQLiteExec` if you want to do joins, or more queries on the same set of parquet files, etc.
* `ParquetSQLiteCloseDatabase(outParamSQLitePointer)`

Note: `ParquetSQLiteFreeMemory` is there only for the `errorMessage` parameter memory clean up when there is an error in `ParquetSQLiteExec`

## Supported features

### Row group filtering

Row group filtering is supported for strings and numerics so long as the SQLite
type matches the Parquet type.

e.g. if you have a column `foo` that is an INT32, this query will skip row groups whose
statistics prove that it does not contain relevant rows:

```
SELECT * FROM tbl WHERE foo = 123;
```

but this query will devolve to a table scan:

```
SELECT * FROM tbl WHERE foo = '123';
```

This is laziness on my part and could be fixed without too much effort.

### Row filtering

For common constraints, the row is checked to see if it satisfies the query's
constraints before returning control to SQLite's virtual machine. This minimizes
the number of allocations performed when many rows are filtered out by
the user's criteria.

### Memoized slices

Individual clauses are mapped to the row groups they match.

eg going on row group statistics, which store minimum and maximum values, a clause
like `WHERE city = 'Dawson Creek'` may match 80% of row groups.

In reality, it may only be present in one or two row groups.

This is recorded in a shadow table so future queries that contain that clause
can read only the necessary row groups.

### Types

These Parquet types are supported:

* INT96 timestamps (exposed as milliseconds since the epoch)
* INT8/INT16/INT32/INT64
* UTF8 strings
* BOOLEAN
* FLOAT
* DOUBLE
* Variable- and fixed-length byte arrays

These are not currently supported:

* UINT8/UINT16/UINT32/UINT64
* DECIMAL
