#pragma once

#include <Core/Block.h>
#include <Formats/FormatSettings.h>
#include <Processors/Chunk.h>
#include <Processors/Formats/Impl/Parquet/ParquetColumnReader.h>

#include <arrow/io/interfaces.h>
#include <parquet/arrow/schema.h>
#include <parquet/file_reader.h>
#include <parquet/properties.h>

#include "ParquetColumnReader.h"

namespace DB
{

struct SelectQueryInfo;
struct PrewhereInfo;
struct PrewhereExprInfo;
class ArrowFieldIndexUtil;
class BlockMissingValues;

class ParquetRecordReader
{
public:
    ParquetRecordReader(
        Block header_,
        std::shared_ptr<arrow::io::RandomAccessFile> source_,
        std::unique_ptr<parquet::ParquetFileReader> file_reader_,
        const parquet::ArrowReaderProperties & reader_properties_,
        const ArrowFieldIndexUtil & field_util_,
        const FormatSettings & format_settings,
        std::vector<int> row_groups_indices_,
        std::shared_ptr<PrewhereInfo> prewhere_info_);

    ~ParquetRecordReader();

    Chunk readChunk(BlockMissingValues * block_missing_values);

    // follow the scale generated by spark
    static constexpr UInt8 default_datetime64_scale = 9;

private:
    std::unique_ptr<ParquetColumnReader> createColReader(
        const ColumnWithTypeAndName & ch_column,
        const std::vector<int> & arrow_col_indicies);

    std::unique_ptr<ParquetColumnReader> createColReader(
        const ColumnWithTypeAndName & ch_column,
        const parquet::arrow::SchemaField & schema_field,
        // const std::vector<int> & leaf_field_indicies,
        const std::vector<int> & arrow_col_indicies);

    Block header;
    std::shared_ptr<arrow::io::RandomAccessFile> source;
    std::unique_ptr<parquet::ParquetFileReader> file_reader;
    parquet::ArrowReaderProperties reader_properties;
    const ArrowFieldIndexUtil & field_util;
    FormatSettings format_settings;
    parquet::arrow::SchemaManifest manifest;

    struct ChunkReader {
        Block sample_block;
        ParquetColReaders column_readers;
        std::vector<bool> default_col_reader;

        std::shared_ptr<PrewhereExprInfo> prewhere_actions;

        ChunkReader() = default;
        explicit ChunkReader(Block sample_block_)
            : sample_block(std::move(sample_block_)) {}

        void init(ParquetRecordReader &record_reader);

        Columns readBatch(size_t num_rows, const IColumn::Filter * filter);
        void skip(size_t num_rows);

        ColumnPtr executePrewhereAction(Block & block, const std::shared_ptr<PrewhereInfo> & prewhere_info);
    };

    ChunkReader active_chunk_reader;
    ChunkReader lazy_chunk_reader;

    std::shared_ptr<PrewhereInfo> prewhere_info;

    std::shared_ptr<parquet::RowGroupReader> cur_row_group_reader;
    UInt64 max_block_size;

    std::vector<int> row_groups_indices;
    size_t cur_row_group_left_rows = 0;
    int next_row_group_idx = 0;

    Poco::Logger * log;
    bool loadNextRowGroup();
};

}
