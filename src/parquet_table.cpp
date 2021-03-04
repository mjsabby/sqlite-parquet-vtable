#include "parquet_table.h"

#include "parquet/api/reader.h"

ParquetTable::ParquetTable(const std::string &tableName, int argc, const char *const *argv)
{
    for (int i = 0; i < argc - 3; ++i)
    {
        std::string fname = argv[i + 3];
        fname = fname.substr(1, fname.length() - 2);
        this->files.push_back(parquet::ParquetFileReader::OpenFile(fname, true, parquet::default_reader_properties()));
        const int fileIndex = static_cast<int>(this->files.size()) - 1;
        const auto &file = this->files[fileIndex];

        const int numRowGroups = file->metadata()->num_row_groups();
        this->totalRowGroups += numRowGroups;
        this->totalRows += file->metadata()->num_rows();

        for (int j = 0; j < numRowGroups; ++j)
        {
            this->indices.emplace_back(fileIndex, j);
        }
    }

    this->tableName = tableName;
    this->CreateStatement(this->files[0]->metadata()->schema());
}

std::string ParquetTable::columnName(int i)
{
    if (i == -1)
        return "rowid";
    return columnNames[i];
}

unsigned int ParquetTable::getNumColumns()
{
    return columnNames.size();
}

const parquet::RowGroupMetaData *ParquetTable::getRowGroupMetaData(int index)
{
    const auto f = this->indices[index];
    return this->files[f.FileIndex]->RowGroup(f.RowGroupIndex)->metadata();
}

std::shared_ptr<parquet::RowGroupReader> ParquetTable::getRowGroupReader(int index)
{
    const auto f = this->indices[index];
    return this->files[f.FileIndex]->RowGroup(f.RowGroupIndex);
}

const parquet::ParquetFileReader *ParquetTable::getFileReader(int index)
{
    const auto f = this->indices[index];
    return this->files[f.FileIndex].get();
}

std::string ParquetTable::getCreateStatement()
{
    return this->create;
}

void ParquetTable::CreateStatement(const parquet::SchemaDescriptor *schema)
{
    std::string text("CREATE TABLE x(");

    for (auto i = 0; i < schema->num_columns(); i++)
    {
        auto _col = schema->GetColumnRoot(i);
        columnNames.push_back(_col->name());
    }

    columnNames.push_back("VirtualRowId");

    for (auto i = 0; i < schema->num_columns(); i++)
    {
        auto _col = schema->GetColumnRoot(i);

        if (!_col->is_primitive())
        {
            std::ostringstream ss;
            ss << __FILE__ << ":" << __LINE__ << ": column " << i << " has non-primitive type";
            throw std::invalid_argument(ss.str());
        }

        /* TODO: Do something better than just ignoring */
        /*
        if (_col->is_repeated())
        {
            std::ostringstream ss;
            ss << __FILE__ << ":" << __LINE__ << ": column " << i << " has non-scalar type";
            throw std::invalid_argument(ss.str());
        }
        */

        parquet::schema::PrimitiveNode *col = (parquet::schema::PrimitiveNode *)_col;

        if (i > 0)
            text += ", ";

        text += "\"";
        // Horrifically inefficient, but easy to understand.
        std::string colName = col->name();
        for (char &c : colName)
        {
            if (c == '"')
                text += "\"\"";
            else
                text += c;
        }
        text += "\"";

        std::string type;

        parquet::Type::type physical = col->physical_type();
        const auto &logical = col->logical_type();
        // Be explicit about which types we understand so we don't mislead someone
        // whose unsigned ints start getting interpreted as signed. (We could
        // support this for UINT_8/16/32 -- and for UINT_64 we could throw if
        // the high bit was set.)
        if (logical->is_none() || logical->is_string() || logical->is_date() || logical->is_time() || logical->is_timestamp() ||
            logical->is_int())
        {
            switch (physical)
            {
            case parquet::Type::BOOLEAN:
                type = "TINYINT";
                break;
            case parquet::Type::INT32: {
                const auto &integerType = arrow::internal::checked_pointer_cast<const parquet::IntLogicalType>(logical);
                if (logical->is_none() || (integerType->is_signed() && integerType->bit_width() == 32))
                {
                    type = "INT";
                }
                else if (integerType->is_signed() && integerType->bit_width() == 8)
                {
                    type = "TINYINT";
                }
                else if (integerType->is_signed() && integerType->bit_width() == 16)
                {
                    type = "SMALLINT";
                }
                break;
            }
            case parquet::Type::INT96:
                // INT96 is used for nanosecond precision on timestamps; we truncate
                // to millisecond precision.
            case parquet::Type::INT64:
                type = "BIGINT";
                break;
            case parquet::Type::FLOAT:
                type = "REAL";
                break;
            case parquet::Type::DOUBLE:
                type = "DOUBLE";
                break;
            case parquet::Type::BYTE_ARRAY:
                if (logical->is_string())
                {
                    type = "TEXT";
                }
                else
                {
                    type = "BLOB";
                }
                break;
            case parquet::Type::FIXED_LEN_BYTE_ARRAY:
                type = "BLOB";
                break;
            default:
                break;
            }
        }

        if (type.empty())
        {
            std::ostringstream ss;
            ss << __FILE__ << ":" << __LINE__ << ": column " << i << " has unsupported type: " << parquet::TypeToString(physical) << "/"
               << logical->ToString();

            throw std::invalid_argument(ss.str());
        }

#ifdef DEBUG
        printf("col %d[name=%s, p=%d:%s, l=%d:%s] is %s\n", i, col->name().data(), col->physical_type(),
               parquet::TypeToString(col->physical_type()).data(), col->logical_type(),
               parquet::LogicalTypeToString(col->logical_type()).data(), type.data());
#endif

        text += " ";
        text += type;
    }

    text += ", \"VirtualRowId\" INT";

    text += ");";
    this->create = text;
}

const std::string &ParquetTable::getTableName()
{
    return tableName;
}
