#pragma once

#include <optional>
#include <sqlite3.h>
#include <string>
#include <tuple>
#include <utility>

class Statement
{
public:
    Statement() = default;
    Statement(sqlite3_stmt* stmt, const char* sql, sqlite3* db);
    Statement(Statement&& other);
    Statement& operator=(Statement&& other);
    ~Statement();

    // Rearm the statement to be bound and executed again
    void reset();

    void bind(int index, const std::string& text);
    void bind(int index, int value);

    // Use this overload if no contents are desired and you wish to simply execute the statement
    bool step();

    // Use this overload if you wish to extract the values of the query as a type-safe tuple
    // e.g. auto result = statement.step<int, std::string>();
    //      will execute the statement and return a tuple assuming column 1 holds an int, and column 2 holds a blob
    // If a row is returned, result will evaluate to true and you can extract the contents conveniently using
    // structured bindings like so:
    // if (result)
    // {
    //     auto&& [v1, v2] = *result;
    // }
    template <typename... Ts> std::optional<std::tuple<Ts...>> step()
    {
        using Tuple = std::tuple<Ts...>;
        using Sequence = std::make_index_sequence<sizeof...(Ts)>;

        int result = sqlite3_step(stmt_);
        check_error(result);
        if (result != SQLITE_ROW)
        {
            return {};
        }

        return {extract<Ts...>(Sequence{})};
    }

    // Helper to zip column types with 0-indexed sequence
    template <typename... Ts, typename T, size_t... Is> std::tuple<Ts...> extract(std::integer_sequence<T, Is...>)
    {
        return std::make_tuple(extract<Ts, Is>()...);
    }

    template <typename T, size_t I> T extract()
    {
        return extract<T>(I);
    }

    // Template specializations of T are explicitly defined in the translation unit Statement.cpp
    template <typename T> T extract(size_t index);

    [[nodiscard]] sqlite3_stmt* handle() const noexcept
    {
        return stmt_;
    }

private:
    void check_error(int result);
    void check(int result);

    sqlite3_stmt* stmt_ = nullptr;
    const char* sql_ = "";
    sqlite3* db_ = nullptr;
};

