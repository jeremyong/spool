#pragma once

#include "Statement.hpp"
#include <cstdio>
#include <vector>

class Database;
class Generator
{
public:
    Generator(Database& db, std::FILE* fp);

    void write_strings();
    void write_source_chunks();
    void write_offsets();

private:
    Database& db_;
    std::FILE* fp_;
    std::vector<int> offsets_;
};

