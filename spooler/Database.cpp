#include "Database.hpp"

#include <iostream>
#include <stdexcept>


Database::Database(const char* path)
{
    int result = sqlite3_open(path, &db_);
    if (result)
    {
        std::cerr << "Failed to open db: " << path << '\n';
        throw std::runtime_error("Failed to open db");
        return;
    }
}

Database::~Database()
{
    if (locked_)
    {
        sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr);
    }
    sqlite3_close(db_);
}

Statement Database::prepare(const char* statement)
{
    sqlite3_stmt* out = nullptr;
    // https://www.sqlite.org/c3ref/prepare.html
    int result = sqlite3_prepare_v2(db_, statement, -1, &out, nullptr);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statment: " << statement << '\n';
        std::cerr << sqlite3_errmsg(db_) << '\n';
        throw std::runtime_error("Failed to prepare statement");
    }
    return {out, statement, db_};
}

int Database::last_insert_rowid()
{
    return sqlite3_last_insert_rowid(db_);
}

void Database::lock()
{
    if (locked_)
    {
        return;
    }

    char* error;
    // https://www.sqlite.org/c3ref/exec.html
    int result = sqlite3_exec(db_, "PRAGMA locking_mode = EXCLUSIVE; BEGIN EXCLUSIVE;", nullptr, nullptr, &error);
    if (result != SQLITE_OK)
    {
        std::cerr << "Failed to acquire database lock (" << result << "): " << error << std::endl;
        throw std::runtime_error("Failed to acquire db lock");
    }
    locked_ = true;
}
