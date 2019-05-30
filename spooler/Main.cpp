#include "Database.hpp"
#include "Generator.hpp"
#include "Origins.hpp"
#include "Parser.hpp"
#include "Strings.hpp"
#include <cstdio>
#include <cstring>
#include <sqlite3.h>
#include <string>
#include <vector>

void print_help()
{
    printf(
        "Usage:\n"
        "spooler [command] [path to db] [path to file] [macro name]\n"
        "\n"
        "where [command] is one of:\n"
        "  - analyze: Given a database and a file, extract literal dependencies for pooling later\n"
        "  - generate: Given a database of strings, emit the finalized spool sources\n"
        "\n"
        "The final macro name argument is used to customize how pooled string literals should be denoted\n"
        "\n");
}

int finalize(Database& db, const char* file_path, const char* macro_name)
{
    std::FILE* fp = std::fopen(file_path, "wb");
    if (!fp)
    {
        fprintf(stderr, "Failed to open file for writing: %s", file_path);
        return 1;
    }

    db.lock();
    Generator generator{db, fp};
    generator.write_strings();
    generator.write_source_chunks();
    generator.write_offsets();

    return 0;
}

int analyze(Database& db, const char* file_path, int source_id, const char* macro_name)
{
    std::FILE* fp = std::fopen(file_path, "rb");
    if (!fp)
    {
        fprintf(stderr, "Failed to open file for reading: %s", file_path);
        return 1;
    }

    // Query file size
    std::fseek(fp, 0, SEEK_END);
    size_t size = std::ftell(fp);
    std::fseek(fp, 0, SEEK_SET);
    // Read all contents (employ `new` so as to avoid the hassle of initiailizing memory we're about to overwrite)
    char* contents = new char[size + 2];
    std::fread(contents + 1, 1, size, fp);
    // Pad both sides with null bytes for convenient parsing
    contents[0] = '\0';
    contents[size + 1] = '\0';
    printf("%s", contents);

    Parser parser(contents, size + 2, macro_name);
    parser.parse();

    delete[] contents;

    db.lock();
    Origins origins(db, source_id);
    origins.select();
    // Copy here is intentional
    auto& counts = origins.ref_counts();
    std::unordered_map<int, int> new_counts;

    Strings strings(db);

    for (auto& [id, ref_count] : counts)
    {
        strings.lookup_id(id);
        strings.dec_by(id, ref_count);
    }

    std::vector<int> ids;

    for (auto& str : parser.literals())
    {
        // Fetch existing ref counts or initialize
        auto id = strings.id(str);
        strings.inc(id);
        ids.emplace_back(id);
        ++new_counts[id];
    }

    strings.commit();
    origins.commit_new(new_counts);

    Statement nuke_old = db.prepare("DELETE FROM flat_offsets WHERE path_id = ?;");
    nuke_old.bind(1, source_id);
    nuke_old.step();
    nuke_old.reset();

    Statement inserter = db.prepare("INSERT INTO flat_offsets (path_id, id) VALUES (?, ?);");
    for (auto id : ids)
    {
        inserter.bind(1, source_id);
        inserter.bind(2, id);
        inserter.step();
        inserter.reset();
    }

    return 0;
}

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        print_help();
        return 0;
    }

    const char* db_path = argv[2];
    const char* file_path = argv[3];
    const char* macro_name = argv[4];

    // Open database connection
    Database db{db_path};

    int result;

    if (strcmp(argv[1], "generate") == 0)
    {
        result = finalize(db, file_path, macro_name);
    }
    else if (strcmp(argv[1], "analyze") == 0)
    {
        int source_id = std::stoi(argv[5]);
        result = analyze(db, file_path, source_id, macro_name);
    }
    else
    {
        fprintf(stderr, "Unrecognized command: %s", argv[2]);
    }

    return result;
}
