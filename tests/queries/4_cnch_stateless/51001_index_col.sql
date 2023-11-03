DROP TABLE IF EXISTS test.index_table;

CREATE TABLE test.index_table
(
    `ts` DateTime64(3),
    `message` String,
    INDEX ts ts TYPE minmax GRANULARITY 4,
    INDEX message message TYPE tokenbf_v1(32768, 2, 0) GRANULARITY 2
)
ENGINE = CnchMergeTree
PARTITION BY toStartOfInterval(ts, toIntervalHour(12))
ORDER BY ts
SETTINGS index_granularity = 8;

INSERT INTO test.index_table VALUES ('2023-10-17 00:11:58.996', '2015-01-04'),('2023-10-17 00:12:58.996', '2016-01-04'),('2023-10-17 00:13:58.996', '2017-01-04'),('2023-10-17 00:14:58.996', '2018-01-04'),('2023-10-17 00:15:58.996', '2019-01-04'),('2023-10-17 00:16:58.996', '2020-01-04'),('2023-10-17 00:17:58.996', '2021-01-04'),('2023-10-17 00:18:58.996', '2024-01-04'),('2023-10-17 00:19:58.996', '2034-01-04'),('2023-10-17 00:20:58.996', '2044-01-04'),('2023-10-17 00:21:58.996', '2014-04-04'),('2023-10-17 00:31:58.996', '2014-05-04'),('2023-10-17 00:41:58.996', '2014-07-04'),('2023-10-17 00:51:58.996', '2014-08-04'),('2023-10-17 00:61:58.996', '2014-09-04'),('2023-10-17 00:71:58.996', '2014-01-01'),('2023-10-17 00:81:58.996', '2014-01-02'),('2023-10-17 00:91:58.996', '2014-01-03'),('2023-10-17 01:11:58.996', '2014-01-04'),('2023-10-17 02:11:58.996', '2014-01-04'),('2023-10-17 03:11:58.996', '2014-01-04'),('2023-10-17 04:11:58.996', '2014-01-05'),('2023-10-17 05:11:58.996', '2014-01-05'),('2023-10-17 06:11:58.996', '2014-01-03'),('2023-10-17 07:11:58.996', '2014-01-07'),('2023-10-17 08:11:58.996', '2014-01-02'),('2023-10-17 09:11:58.996', '2014-01-01'),('2023-10-17 10:11:58.996', '2014-01-08');

select message from test.index_table where ts = '2023-10-17 00:11:58.996';
