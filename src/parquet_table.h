#ifndef PARQUET_TABLE_H
#define PARQUET_TABLE_H

#define ARROW_STATIC
#define PARQUET_STATIC
#define UTF8PROC_STATIC

#include "parquet/api/reader.h"
#include <string>
#include <vector>

struct FileIndexWithRowGroupInfo
{
    FileIndexWithRowGroupInfo(int fileIndex, int rowGroupIndex)
    {
        this->FileIndex = fileIndex;
        this->RowGroupIndex = rowGroupIndex;
    }

    int FileIndex;
    int RowGroupIndex;
};

class ParquetTable
{
    std::string tableName;
    std::vector<std::unique_ptr<parquet::ParquetFileReader>> files;
    std::vector<FileIndexWithRowGroupInfo> indices;
    std::vector<std::string> columnNames;
    int64_t totalRows = 0;
    int64_t totalRowGroups = 0;
    std::string create;

  private:
    void CreateStatement(const parquet::SchemaDescriptor *schema);

  public:
    ParquetTable(const std::string &tableName, int argc, const char *const *argv);
    std::string columnName(int idx);
    unsigned int getNumColumns();
    const std::string &getTableName();
    int64_t getNumRows() const
    {
        return totalRows;
    }
    int64_t getNumRowGroups() const
    {
        return totalRowGroups;
    }
    const parquet::RowGroupMetaData *getRowGroupMetaData(int index);
    std::shared_ptr<parquet::RowGroupReader> getRowGroupReader(int index);
    const parquet::ParquetFileReader *getFileReader(int index);
    std::string getCreateStatement();
};

#endif
