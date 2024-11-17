#pragma once

#include <cstdint>
#include <sqlite3.h>

#include <concepts>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace sqlight
{

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


    class db final
    {

    public:
        explicit db(std::string_view db_file)
        {
            const auto error = sqlite3_open(db_file.data(), &db_ptr); // TODO: Use v2
            if (error != 0)
            {
                sqlite3_close(db_ptr);
            }
        }

        template <typename... Args, template <typename...> typename Query_t, typename... QueryArgs>
            requires std::same_as<Query_t<QueryArgs...>, query<QueryArgs...>>
        [[maybe_unused]] std::vector<std::tuple<Args...>> execute(const Query_t<QueryArgs...>& sql_query)
        {

            sqlite3_stmt* statement = nullptr;
            sqlite3_prepare_v2(db_ptr, sql_query.get_query_string().data(),
                               static_cast<int>(sql_query.get_query_string().size()), &statement, nullptr);

            [&]<auto... Is>(std::index_sequence<Is...>) {
                ((bind_param(statement, std::get<Is>(sql_query.get_query_args()), Is + 1)), ...);
            }(std::index_sequence_for<QueryArgs...>{});

            auto step = sqlite3_step(statement);

            std::vector<std::tuple<Args...>> results;

            while (step == SQLITE_ROW)
            {
                auto row = std::tuple<Args...>{};
                [&]<auto... Is>(std::index_sequence<Is...>)
                { ((get_column(statement, Is, std::get<Is>(row))), ...); }(std::index_sequence_for<Args...>{});

                results.emplace_back(row);

                step = sqlite3_step(statement);
            }

            sqlite3_finalize(statement);

            return results;
        }

        template <typename... Args, typename... QueryArgs>
        [[maybe_unused]] std::vector<std::tuple<Args...>> execute(std::string_view sql_query, QueryArgs&&... args)
        {
            return execute<Args...>(query{ std::string{ sql_query }, args... });
        }

        ~db()
        {
            if (db_ptr == nullptr)
            {
                return;
            }
            sqlite3_close(db_ptr);
        }

    private:
        template <typename T>
        static void bind_param(sqlite3_stmt* statement, T&& arg, int index)
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
        }

        template <typename T>
        void get_column(sqlite3_stmt* statement, int index, T& value)
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

        sqlite3* db_ptr = nullptr;
    };

} // namespace sqlight
