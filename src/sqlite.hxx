#include <functional>
#include <list>
#include <optional>
#include <utility>

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
    auto prepare(std::string_view query) -> Stmt;
    auto prepare_all(std::string_view query) -> std::list<Stmt>;
    template<typename... TS>
    auto prepare_bind(std::string_view query, TS... binds) -> Stmt;
    auto exec(std::string_view query) -> void;
};

class SQLite::Stmt {
public:
    const SQLite &db;
    sqlite3_stmt *stmt;

    Stmt(const SQLite &, sqlite3_stmt *);
    ~Stmt();

    auto error() const noexcept -> Error;

    auto bind(int id, int) -> void;
    auto bind(int id, int64_t) -> void;
    auto bind(int id, std::string_view) -> void;
    template<typename T, typename... TS>
    auto bind(int start_id, T first_bind, TS... rest) -> void;

    auto step() -> std::optional<Row>;
    auto exec() -> void;
    template<typename F>
    auto iterate(F &&f) -> void;
};

class SQLite::Row {
public:
    const Stmt &stmt;

    Row(const Stmt &);
    ~Row();

    auto column_int(int) -> int;
    auto column_int64(int) -> int64_t;
    /* WARNING: resulting `string_view` validity */
    auto column_text(int) -> std::string_view;

private:
    auto _call(int, std::function<auto() -> void>) -> void;
    template<typename... TS>
    auto _call(int, std::function<auto(int, TS...) -> void>) -> void;
    template<typename... TS>
    auto _call(int, std::function<auto(int64_t, TS...) -> void>) -> void;
    template<typename... TS>
    auto _call(int, std::function<auto(std::optional<std::string_view>, TS...) -> void>) -> void;

public:
    template<typename F>
    auto call(int n, F &&f) -> void;
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
auto SQLite::prepare_bind(std::string_view query, TS... binds) -> Stmt {
    auto stmt = this->prepare(query);
    stmt.bind(1, binds...);
    return stmt;
}

template<typename T, typename... TS>
auto SQLite::Stmt::bind(int id, T first_bind, TS... rest) -> void {
    this->bind(id, first_bind);
    this->bind(id + 1, rest...);
}

template<typename F>
auto SQLite::Stmt::iterate(F &&f) -> void {
    while (auto r = this->step())
        r->call(0, std::forward<F>(f));
}

template<typename F>
auto SQLite::Row::call(int n, F &&f) -> void {
        this->_call(n, std::function(f));
}

inline auto SQLite::Row::_call(
    [[maybe_unused]] int n,
    std::function<auto() -> void> f
) -> void {
    f();
}

template<typename... TS>
auto SQLite::Row::_call(
    int n,
    std::function<auto(int, TS...) -> void> f
) -> void {
    auto c = this->column_int(n);
    this->call(n + 1, [=](TS... args) -> void {
        f(c, args...);
    });
}

template<typename... TS>
auto SQLite::Row::_call(
    int n,
    std::function<auto(int64_t, TS...) -> void> f
) -> void {
    auto c = this->column_int64(n);
    this->call(n + 1, [=](TS... args) -> void {
        f(c, args...);
    });
}

template<typename... TS>
auto SQLite::Row::_call(
    int n,
    std::function<auto(std::optional<std::string_view>, TS...) -> void> f
) -> void {
    auto c = this->column_text(n);
    this->call(n + 1, [=](TS... args) -> void {
        f(c, args...);
    });
}
