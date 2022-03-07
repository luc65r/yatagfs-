#include <optional>

#include <sqlite3.h>
#include "carray.h"

class SQLite {
public:
    sqlite3 *db;

    class Stmt;
    class Error;
    class Row;

    SQLite(const char *path);
    ~SQLite();

    auto error() const noexcept -> Error;
    auto prepare(const char *query) -> Stmt;
    template<typename... TS>
    auto prepare_bind(const char *query, TS... binds) -> Stmt;
};

class SQLite::Stmt {
public:
    const SQLite *db;
    sqlite3_stmt *stmt;

    Stmt(const SQLite *, sqlite3_stmt *);
    ~Stmt();

    auto error() const noexcept -> Error;

    auto bind(int id, int) -> void;
    auto bind(int id, int64_t) -> void;
    auto bind(int id, const char *) -> void;
    template<typename T, typename... TS>
    auto bind_all(int start_id, T first_bind, TS... rest) -> void;

    auto step() -> std::optional<Row>;
};

class SQLite::Row {
public:
    const Stmt *stmt;

    Row(const Stmt *);
    ~Row();

    auto column_int(int) -> int;
    auto column_int64(int) -> int64_t;
    auto column_text(int) -> const unsigned char *;
};

class SQLite::Error : public std::exception {
public:
    std::string msg;

    Error(int) noexcept;
    Error(sqlite3 *) noexcept;
    Error(const char *) noexcept;
    ~Error();

    auto what() const noexcept -> const char * override;
};

template<typename... TS>
auto SQLite::prepare_bind(const char *query, TS... binds) -> Stmt {
    auto stmt = this->prepare(query);
    stmt.bind_all(1, binds...);
    return stmt;
}

template<typename T, typename... TS>
auto SQLite::Stmt::bind_all(int id, T first_bind, TS... rest) -> void {
    this->bind(id, first_bind);
    this->bind_all(id + 1, rest...);
}
