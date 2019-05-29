#include "Origins.hpp"
#include "Database.hpp"

#include <cstring>
#include <stdexcept>
#include <thread>

static const char* query = "SELECT id, ref_count from origins WHERE path_id = ?;";
static const char* insert = "INSERT INTO origins (path_id, id, ref_count) VALUES (?, ?, ?);";
// Avoid redefinition of remove
static const char* remove_sql = "DELETE FROM origins WHERE path_id = ? AND id = ?;";
static const char* update = "UPDATE origins SET ref_count = ? WHERE path_id = ? AND id = ?;";

Origins::Origins(Database& db, int source_id)
    : db_{db}
    , source_id_{source_id}
{
    // https://www.sqlite.org/c3ref/prepare.html
    query_ = db.prepare(query);
    insert_ = db.prepare(insert);
    remove_ = db.prepare(remove_sql);
    update_ = db.prepare(update);
}

void Origins::select()
{
    query_.bind(1, source_id_);
    while (auto result = query_.step<int, int>())
    {
        auto&& [id, ref_count] = *result;
        ref_counts_[id] = ref_count;
    }
    query_.reset();
}

void Origins::commit_new(std::unordered_map<int, int>& counts)
{
    for (auto iter = ref_counts_.begin(); iter != ref_counts_.end(); ++iter)
    {
        auto next = counts.find(iter->first);
        if (next == counts.end())
        {
            // This string needs to be removed
            remove_.bind(1, source_id_);
            remove_.bind(2, iter->first);
            remove_.step();
            remove_.reset();
        }
        else if (iter->second != next->second)
        {
            // The ref count needs to be updated
            update_.bind(1, next->second);
            update_.bind(2, source_id_);
            update_.bind(3, next->first);
            update_.step();
            update_.reset();

            counts.erase(next);
        }
        else
        {
            // Count is unchanged
            counts.erase(next);
        }
    }

    // Remaining elements of counts now all need to be newly added
    for (auto&& [id, count] : counts)
    {
        insert_.bind(1, source_id_);
        insert_.bind(2, id);
        insert_.bind(3, count);
        insert_.step();
        insert_.reset();
    }
}

int Origins::file_id()
{
    // Query for an existing file id, or create a new one otherwise
    Statement insert_file = db_.prepare("INSERT INTO sources (path) VALUES (?);");
    // If the file exists already, a new one won't be created due to the unique constraint
    insert_file.bind(1, source_id_);
    insert_file.step();
    insert_file.reset();

    Statement select_file = db_.prepare("SELECT ROWID FROM sources WHERE path = ?;");
    select_file.bind(1, source_id_);
    auto&& [result] = *select_file.step<int>();
    select_file.reset();
    return result;
}

