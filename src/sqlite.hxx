#include <list>
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
    auto prepare_all(const char *query) -> std::list<Stmt>;
    template<typename... TS>
    auto prepare_bind(const char *query, TS... binds) -> Stmt;
    auto exec(const char *query) -> void;
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
    auto bind(int start_id, T first_bind, TS... rest) -> void;

    auto step() -> std::optional<Row>;
    auto exec() -> void;
};

class SQLite::Row {
public:
    const Stmt *stmt;

    Row(const Stmt *);
    ~Row();

    auto column_int(int) -> int;
    auto column_int64(int) -> int64_t;
    auto column_text(int) -> const char *;
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
    stmt.bind(1, binds...);
    return stmt;
}

template<typename T, typename... TS>
auto SQLite::Stmt::bind(int id, T first_bind, TS... rest) -> void {
    this->bind(id, first_bind);
    this->bind(id + 1, rest...);
}
