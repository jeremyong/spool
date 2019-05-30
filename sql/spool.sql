-- Map from strings to index into the global string literal array
CREATE TABLE IF NOT EXISTS strings (
    id INT PRIMARY KEY,
    string TEXT NOT NULL UNIQUE,
    ref_count INT UNSIGNED NOT NULL DEFAULT 0
);

-- Record where strings are used to track ref counts across successive compiles
CREATE TABLE IF NOT EXISTS origins (
    path_id INT NOT NULL,
    id INT UNSIGNED NOT NULL,
    ref_count INT UNSIGNED NOT NULL,
    PRIMARY KEY (path_id, id)
);

CREATE TABLE IF NOT EXISTS flat_offsets (
    path_id INT NOT NULL,
    id INT UNSIGNED NOT NULL
);

CREATE INDEX IF NOT EXISTS path_id_index ON flat_offsets (path_id);
