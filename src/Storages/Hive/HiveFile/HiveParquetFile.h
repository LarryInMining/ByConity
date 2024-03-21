#pragma once

#include "Common/config.h"
#if USE_HIVE

#include "Processors/ISource.h"
#include "Storages/Hive/HiveFile/IHiveFile.h"

namespace parquet::arrow { class FileReader; }
namespace parquet { class FileMetaData; }
namespace arrow { class Schema; }

namespace DB
{
class ArrowColumnToCHColumn;

class HiveParquetFile : public IHiveFile
{
public:
    Features getFeatures() const override
    {
        return Features{
            .support_file_splits = true,
            .support_file_minmax_index = false,
            .support_split_minmax_index = true,
        };
    }

    HiveParquetFile();
    ~HiveParquetFile() override;

    size_t numSlices() const override;
    std::optional<size_t> numRows() const override;

    SourcePtr getReader(const Block & block, const std::shared_ptr<ReadParams> & params) override;

private:
    /// void loadFileMinMaxIndex(const NamesAndTypesList & index_names_and_types) override;
    void loadSplitMinMaxIndex(const NamesAndTypesList & index_names_and_types) override;

    void openFile() const;
    mutable std::unique_ptr<parquet::arrow::FileReader> file_reader;
    mutable std::unique_ptr<ReadBuffer> buf;
    mutable std::shared_ptr<parquet::FileMetaData> metadata;
    mutable std::shared_ptr<arrow::Schema> schema;
    mutable std::mutex mutex;
};

}
#endif
