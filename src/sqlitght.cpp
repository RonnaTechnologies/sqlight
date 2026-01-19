#include "sqlight.hpp"

#include <stdexcept>

namespace sqlight
{

    db::db(const std::string& db_file, int flags)
    {
        const auto error = sqlite3_open_v2(db_file.data(), &db_ptr, flags, nullptr);
        if (error != SQLITE_OK)
        {
            std::string error_message = db_ptr ? sqlite3_errmsg(db_ptr) : "Failed to open database";
            sqlite3_close(db_ptr);
            throw std::runtime_error{ error_message };
        }
    }

    db::~db() noexcept
    {
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

    transaction::transaction(std::shared_ptr<db> db_) : committed{ false }, db_ptr{ db_ }
    {
        db_ptr->execute("BEGIN TRANSACTION;");
    }

    transaction::~transaction()
    {
        if (!committed)
        {
            try
            {
                db_ptr->execute("ROLLBACK;");
            }
            catch (...)
            {
            }
        }
    }

    void transaction::commit()
    {
        db_ptr->execute("COMMIT;");
        committed = true;
    }

} // namespace sqlight