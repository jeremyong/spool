#pragma once

#include "Statement.hpp"
#include <sqlite3.h>
#include <unordered_map>

class Database;
class Strings
{
public:
    Strings(Database& db);
    Strings(const Strings&) = delete;
    Strings(Strings&&) = delete;
    Strings& operator=(const Strings&) = delete;
    Strings& operator=(Strings&&) = delete;

    void lookup_id(int id);
    int id(const std::string& str);
    void inc(int id);
    void dec_by(int id, int d);

    void commit();

private:
    Database& db_;
    Statement query_;
    Statement query_by_id_;
    Statement insert_;
    Statement delta_;
    Statement remove_;
    std::unordered_map<int, int> deltas_;
    std::unordered_map<int, int> ref_counts_;
    std::unordered_map<std::string, int> ids_;
};

