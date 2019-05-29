#pragma once

#include "Statement.hpp"
#include <sqlite3.h>

class Database
{
public:
    Database(const char* path);
    Database(const Database&) = delete;
    Database(Database&&) = delete;
    Database& operator=(const Database&) = delete;
    Database& operator=(Database&&) = delete;
    ~Database();

    Statement prepare(const char* statement);
    int last_insert_rowid();

    void lock();
    [[nodiscard]] sqlite3* handle() const noexcept
    {
        return db_;
    }

private:
    sqlite3* db_;
    bool locked_ = false;
};

