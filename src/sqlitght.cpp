#include "sqlight.hpp"

#include <stdexcept>

namespace sqlight
{

    db::db(std::string_view db_file)
    {
        const auto error = sqlite3_open(db_file.data(), &db_ptr); // TODO: Use v2
        if (error != 0)
        {
            sqlite3_close(db_ptr);
            db_ptr = nullptr;
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
        auto db_sptr = weak_from_this().lock();
        if (!db_sptr)
        {
            throw std::runtime_error(
            "Error: transaction can only be created from a shared_ptr to a database connection.");
        }
        return sqlight::transaction{ db_sptr };
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