#include "Strings.hpp"
#include "Database.hpp"

#include <stdexcept>

static const char* query_by_id = "SELECT ROWID, ref_count FROM strings WHERE ROWID = ?;";
static const char* query = "SELECT ROWID, ref_count FROM strings WHERE string = ?;";
static const char* delta = "UPDATE strings SET ref_count = ref_count + ? WHERE ROWID = ?;";
static const char* insert = "INSERT INTO strings (string) VALUES (?);";
static const char* remove_sql = "DELETE FROM strings WHERE ROWID = ?;";

Strings::Strings(Database& db)
    : db_{db}
{
    query_ = db.prepare(query);
    query_by_id_ = db.prepare(query_by_id);
    delta_ = db.prepare(delta);
    insert_ = db.prepare(insert);
    remove_ = db.prepare(remove_sql);
}

void Strings::lookup_id(int id)
{
    query_by_id_.bind(1, id);
    auto result = query_by_id_.step<int, int>();
    if (result)
    {
        auto&& [id, ref_count] = *result;
        ref_counts_[id] = ref_count;
        query_by_id_.reset();
    }
    else
    {
        throw std::runtime_error("DB inconsistency detected. Consider rebuilding spool database.");
    }
}

int Strings::id(const std::string& str)
{
    auto iter = ids_.find(str);
    if (iter != ids_.end())
    {
        return iter->second;
    }

    query_.bind(1, str);

    auto result = query_.step<int, int>();

    if (result)
    {
        auto&& [id, ref_count] = *result;
        ref_counts_[id] = ref_count;
        query_.reset();
        return id;
    }
    query_.reset();

    // We need to assign a hole to insert a new string
    int id = -1;

    insert_.bind(1, str);
    insert_.step();
    id = db_.last_insert_rowid();
    insert_.reset();

    ids_[str] = id;
    ref_counts_[id] = 0;
    return id;
}

void Strings::inc(int id)
{
    ++deltas_[id];
}

void Strings::dec_by(int id, int d)
{
    deltas_[id] -= d;
}

void Strings::commit()
{
    for (auto& [id, d] : deltas_)
    {
        if (d == 0)
        {
            continue;
        }

        if (d == -ref_counts_[id])
        {
            // TODO delete string from spool
            remove_.bind(1, id);
            remove_.step();
            remove_.reset();
        }
        else
        {
            delta_.bind(1, d);
            delta_.bind(2, id);
            delta_.step();
            delta_.reset();
        }
    }
}

