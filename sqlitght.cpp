#include "sqlight.hpp"

namespace sqlight
{

    db::db(std::string_view db_file)
    {
        const auto error = sqlite3_open(db_file.data(), &db_ptr); // TODO: Use v2
        if (error != 0)
        {
            sqlite3_close(db_ptr);
        }
    }

    db::~db()
    {
        if (db_ptr == nullptr)
        {
            return;
        }
        sqlite3_close(db_ptr);
    }

    auto db::transaction() -> sqlight::transaction
    {
        return sqlight::transaction{ shared_from_this() };
    }

    transaction::transaction(std::shared_ptr<db> db_) : db_ptr{ db_ }
    {
        db_ptr->execute("BEGIN TRANSACTION;");
    }

    void transaction::commit()
    {
        db_ptr->execute("COMMIT;");
    }

} // namespace sqlight