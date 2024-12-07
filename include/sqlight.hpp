#pragma once

#include <sqlite3.h>

#include <concepts>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace sqlight
{
    static constexpr auto sqlite_version()
    {
        return sqlite3_version;
    }

    template <typename... Args>
    class query final
    {

    public:
        explicit query(std::string query_str, Args... args)
        : query_string{ std::move(query_str) }, query_args{ std::make_tuple(args...) }
        {
        }

        auto get_query_string() const
        {
            return query_string;
        }

        auto get_query_args() const
        {
            return query_args;
        }

    private:
        std::string query_string;
        std::tuple<Args...> query_args;
    };

    class transaction;

    class db final : public std::enable_shared_from_this<db>
    {
        friend class sqlight::transaction;

    public:
        explicit db(std::string_view db_file, int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);

        auto transaction() -> class sqlight::transaction;

        template <typename... Args, template <typename...> typename Query_t, typename... QueryArgs>
            requires std::same_as<Query_t<QueryArgs...>, query<QueryArgs...>>
        [[maybe_unused]] std::vector<std::tuple<Args...>> execute(const Query_t<QueryArgs...>& sql_query) const;

        template <typename... Args, typename... QueryArgs>
        [[maybe_unused]] std::vector<std::tuple<Args...>> execute(std::string_view sql_query, QueryArgs&&... args) const;

        ~db();

    private:
        template <typename T>
        static void bind_param(sqlite3_stmt* statement, T&& arg, int index);

        template <typename T>
        void get_column(sqlite3_stmt* statement, int index, T& value) const;

        sqlite3* db_ptr = nullptr;
    };

    class transaction
    {
    public:
        transaction(std::shared_ptr<db> db_);

        template <typename... Args, template <typename...> typename Query_t, typename... QueryArgs>
            requires std::same_as<Query_t<QueryArgs...>, query<QueryArgs...>>
        [[maybe_unused]] std::vector<std::tuple<Args...>> execute(const Query_t<QueryArgs...>& sql_query) const;


        template <typename... Args, typename... QueryArgs>
        [[maybe_unused]] std::vector<std::tuple<Args...>> execute(std::string_view sql_query, QueryArgs&&... args) const;

        void commit();

        transaction() = delete;
        transaction(transaction&&) = delete;
        auto operator=(transaction&) = delete;
        auto operator=(const transaction&&) = delete;

    private:
        std::shared_ptr<db> db_ptr;
    };

    template <typename... Args, template <typename...> typename Query_t, typename... QueryArgs>
        requires std::same_as<Query_t<QueryArgs...>, query<QueryArgs...>>
    [[maybe_unused]] std::vector<std::tuple<Args...>> db::execute(const Query_t<QueryArgs...>& sql_query) const
    {

        sqlite3_stmt* statement = nullptr;
        const auto error = sqlite3_prepare_v2(db_ptr, sql_query.get_query_string().data(),
                                              static_cast<int>(sql_query.get_query_string().size()), &statement, nullptr);

        [&]<auto... Is>(std::index_sequence<Is...>)
        {
            ((bind_param(statement, std::get<Is>(sql_query.get_query_args()), Is + 1)), ...);
        }(std::index_sequence_for<QueryArgs...>{});

        auto step = sqlite3_step(statement);

        if (step == SQLITE_DONE)
        {
            return {};
        }

        std::vector<std::tuple<Args...>> results;

        while (step == SQLITE_ROW)
        {
            auto row = std::tuple<Args...>{};
            [&]<auto... Is>(std::index_sequence<Is...>)
            { ((get_column(statement, Is, std::get<Is>(row))), ...); }(std::index_sequence_for<Args...>{});

            results.emplace_back(row);

            step = sqlite3_step(statement);
        }

        if (step != SQLITE_DONE)
        {
            throw std::runtime_error{ sqlite3_errmsg(db_ptr) };
        }

        sqlite3_reset(statement);
        sqlite3_finalize(statement);

        return results;
    }

    template <typename... Args, typename... QueryArgs>
    [[maybe_unused]] std::vector<std::tuple<Args...>> db::execute(std::string_view sql_query, QueryArgs&&... args) const
    {
        return execute<Args...>(query{ std::string{ sql_query }, args... });
    }


    template <typename... Args, template <typename...> typename Query_t, typename... QueryArgs>
        requires std::same_as<Query_t<QueryArgs...>, query<QueryArgs...>>
    [[maybe_unused]] std::vector<std::tuple<Args...>> transaction::execute(const Query_t<QueryArgs...>& sql_query) const
    {
        return db_ptr->execute<Args...>(sql_query);
    }

    template <typename... Args, typename... QueryArgs>
    [[maybe_unused]] std::vector<std::tuple<Args...>> transaction::execute(std::string_view sql_query, QueryArgs&&... args) const
    {
        return db_ptr->execute<Args...>(query{ std::string{ sql_query }, args... });
    }

    template <typename T>
    void db::bind_param(sqlite3_stmt* statement, T&& arg, int index)
    {
        using Arg_t = std::remove_cvref_t<T>;

        if constexpr (std::is_same_v<std::int64_t, Arg_t>)
        {
            sqlite3_bind_int64(statement, index, static_cast<std::int64_t>(arg));
        }
        else if constexpr (std::is_integral_v<Arg_t>)
        {
            sqlite3_bind_int(statement, index, static_cast<int>(arg));
        }
        else if constexpr (std::is_same_v<std::string, Arg_t>)
        {
            sqlite3_bind_text(statement, index, arg.data(), arg.size(), SQLITE_TRANSIENT);
        }
    }

    template <typename T>
    void db::get_column(sqlite3_stmt* statement, int index, T& value) const
    {
        if constexpr (std::is_integral_v<T>)
        {
            value = sqlite3_column_int(statement, index);
        }
        if constexpr (std::is_same_v<std::string, T>)
        {
            value = std::string{ reinterpret_cast<const char*>(sqlite3_column_text(statement, index)) };
        }
    }


} // namespace sqlight
