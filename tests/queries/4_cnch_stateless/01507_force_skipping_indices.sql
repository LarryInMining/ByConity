DROP TABLE IF EXISTS data_01507;
CREATE TABLE data_01507
(
    key Int,
    d1 Int,
    d1_null Nullable(Int),
    INDEX d1_idx d1 TYPE minmax GRANULARITY 1,
    INDEX d1_null_idx assumeNotNull(d1_null) TYPE minmax GRANULARITY 1
)
Engine=MergeTree()
ORDER BY key;

INSERT INTO data_01507 VALUES (1, 2, 3);

SELECT * FROM data_01507;
SELECT * FROM data_01507 SETTINGS force_data_skipping_indices=''; -- { serverError 6 }
SELECT * FROM data_01507 SETTINGS force_data_skipping_indices='d1_idx'; -- { serverError 277 }
SELECT * FROM data_01507 SETTINGS force_data_skipping_indices='d1_null_idx'; -- { serverError 277 }

SELECT * FROM data_01507 WHERE d1 = 0 SETTINGS force_data_skipping_indices='d1_idx';
SELECT * FROM data_01507 WHERE d1 = 0 SETTINGS force_data_skipping_indices='`d1_idx`';
SELECT * FROM data_01507 WHERE d1 = 0 SETTINGS force_data_skipping_indices=' d1_idx ';
SELECT * FROM data_01507 WHERE d1 = 0 SETTINGS force_data_skipping_indices='  d1_idx  ';
SELECT * FROM data_01507 WHERE d1 = 0 SETTINGS force_data_skipping_indices='d1_idx,d1_null_idx'; -- { serverError 277 }
SELECT * FROM data_01507 WHERE d1 = 0 SETTINGS force_data_skipping_indices='d1_null_idx,d1_idx'; -- { serverError 277 }
SELECT * FROM data_01507 WHERE d1 = 0 SETTINGS force_data_skipping_indices='d1_null_idx,d1_idx,,'; -- { serverError 277 }
SELECT * FROM data_01507 WHERE d1 = 0 SETTINGS force_data_skipping_indices='  d1_null_idx,d1_idx'; -- { serverError 277 }
SELECT * FROM data_01507 WHERE d1 = 0 SETTINGS force_data_skipping_indices='  `d1_null_idx`,d1_idx'; -- { serverError 277 }
SELECT * FROM data_01507 WHERE d1 = 0 SETTINGS force_data_skipping_indices='d1_null_idx'; -- { serverError 277 }
SELECT * FROM data_01507 WHERE d1 = 0 SETTINGS force_data_skipping_indices='  d1_null_idx  '; -- { serverError 277 }

SELECT * FROM data_01507 WHERE d1_null = 0 SETTINGS force_data_skipping_indices='d1_null_idx'; -- { serverError 277 }
SELECT * FROM data_01507 WHERE assumeNotNull(d1_null) = 0 SETTINGS force_data_skipping_indices='d1_null_idx';

DROP TABLE data_01507;
