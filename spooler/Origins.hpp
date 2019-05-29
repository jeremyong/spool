#pragma once

#include "Statement.hpp"
#include <sqlite3.h>
#include <string>
#include <unordered_map>
#include <vector>

class Database;
class Origins
{
public:
    Origins(Database& db, int source_id);
    Origins(const Origins&) = delete;
    Origins(Origins&&) = delete;
    Origins& operator=(const Origins&) = delete;
    Origins& operator=(Origins&&) = delete;
    void select();
    void commit_new(std::unordered_map<int, int>& counts);
    int file_id();

    [[nodiscard]] const std::unordered_map<int, int>& ref_counts() noexcept
    {
        return ref_counts_;
    }

private:
    Database& db_;
    Statement query_;
    Statement insert_;
    Statement remove_;
    Statement update_;
    int source_id_;
    std::unordered_map<int, int> ref_counts_;
    std::vector<int> ids_;
};

