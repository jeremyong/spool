#include "Statement.hpp"
#include <iostream>
#include <stdexcept>

Statement::Statement(sqlite3_stmt* stmt, const char* sql, sqlite3* db)
    : stmt_{stmt}
    , sql_{sql}
    , db_{db}
{
}

Statement::Statement(Statement&& other)
{
    std::swap(stmt_, other.stmt_);
    std::swap(sql_, other.sql_);
}

Statement& Statement::operator=(Statement&& other)
{
    std::swap(stmt_, other.stmt_);
    std::swap(sql_, other.sql_);
    return *this;
}

Statement::~Statement()
{
    sqlite3_finalize(stmt_);
}

void Statement::bind(int index, const std::string& text)
{
    check(sqlite3_bind_text(stmt_, index, text.data(), text.size(), SQLITE_STATIC));
}

void Statement::bind(int index, int value)
{
    check(sqlite3_bind_int(stmt_, index, value));
}
void Statement::reset()
{
    sqlite3_reset(stmt_);
    sqlite3_clear_bindings(stmt_);
}

bool Statement::step()
{
    int result = sqlite3_step(stmt_);
    check_error(result);
    if (result != SQLITE_DONE)
    {
        return false;
    }

    return true;
}

template <> int Statement::extract<int>(size_t index)
{
    return sqlite3_column_int(stmt_, index);
}

template <> std::string Statement::extract<std::string>(size_t index)
{
    const void* ptr = sqlite3_column_text(stmt_, index);
    size_t size = sqlite3_column_bytes(stmt_, index);

    return {reinterpret_cast<const char*>(ptr), size};
}

void Statement::check_error(int result)
{
    if (result == SQLITE_ERROR || result == SQLITE_MISUSE)
    {
        std::cerr << "Failure (" << sqlite3_errstr(result) << ") executing command for " << sql_ << '\n';
        std::cerr << sqlite3_errmsg(db_) << '\n';
        throw std::runtime_error("DB error");
    }
}

void Statement::check(int result)
{
    if (result != SQLITE_OK)
    {
        std::cerr << "Failure (" << sqlite3_errstr(result) << ") executing command for " << sql_ << '\n';
        std::cerr << sqlite3_errmsg(db_) << '\n';
        throw std::runtime_error("DB error");
    }
}
