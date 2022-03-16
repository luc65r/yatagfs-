#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "sqlite.hxx"

SQLite::SQLite(const char *path) {
    std::cerr << "Opening SQLite database " << path << std::endl;
    int rc = sqlite3_open(path, &db);
    if (rc != SQLITE_OK)
        throw Error(rc);

    char *errormsg;
    rc = sqlite3_carray_init(db, &errormsg, NULL);
    if (rc != SQLITE_OK)
        throw Error(errormsg);
}

SQLite::~SQLite() {
    sqlite3_close(db);
}

auto SQLite::error() const noexcept -> Error {
    return Error(this->db);
}

auto SQLite::prepare(std::string_view query) -> Stmt {
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, query.data(), query.size(), &stmt, NULL))
        throw this->error();

    return Stmt(this, stmt);
}

static auto can_prerpare(std::optional<std::string_view> query) -> bool {
    return query && std::any_of(
        query->begin(), query->end(),
        [](unsigned char c) {
            return !std::isspace(c);
        }
    );
}

auto SQLite::prepare_all(std::string_view query) -> std::list<Stmt> {
    std::optional<std::string_view> remaining = query;
    std::list<Stmt> list;

    while (can_prerpare(remaining)) {
        sqlite3_stmt *stmt;
        std::cerr << "preparing: " << *remaining << std::endl;
        const char *s = nullptr;
        if (sqlite3_prepare_v2(db, remaining->data(), remaining->size(), &stmt, &s))
            throw this->error();
        remaining = s;
        list.emplace_back(this, stmt);
    }

    return list;
}

auto SQLite::exec(std::string_view query) -> void {
    auto stmts = this->prepare_all(query);
    for (auto &&stmt : stmts)
        stmt.exec();
}

SQLite::Stmt::Stmt(const SQLite *db, sqlite3_stmt *stmt)
    : db(db)
    , stmt(stmt)
{}

SQLite::Stmt::~Stmt() {
    sqlite3_finalize(stmt);
}

auto SQLite::Stmt::error() const noexcept -> Error {
    return db->error();
}

auto SQLite::Stmt::bind(int id, int val) -> void {
    if (sqlite3_bind_int(stmt, id, val))
        throw this->error();
}

auto SQLite::Stmt::bind(int id, int64_t val) -> void {
    if (sqlite3_bind_int64(stmt, id, val))
        throw this->error();
}

auto SQLite::Stmt::bind(int id, std::string_view text) -> void {
    if (sqlite3_bind_text(stmt, id, text.data(), text.size(), SQLITE_TRANSIENT))
        throw this->error();
}

auto SQLite::Stmt::step() -> std::optional<Row> {
    switch (sqlite3_step(stmt)) {
    case SQLITE_DONE:
        return {};
    case SQLITE_ROW:
        return Row(this);
    default:
        throw this->error();
    }
}

auto SQLite::Stmt::exec() -> void {
    while (this->step())
        ;
}

SQLite::Row::Row(const Stmt *stmt)
    : stmt(stmt)
{}

SQLite::Row::~Row() = default;

auto SQLite::Row::column_int(int i) -> int {
    return sqlite3_column_int(stmt->stmt, i);
}

auto SQLite::Row::column_int64(int i) -> int64_t {
    return sqlite3_column_int64(stmt->stmt, i);
}

auto SQLite::Row::column_text(int i) -> std::string_view {
    return reinterpret_cast<const char *>(sqlite3_column_text(stmt->stmt, i));
}

SQLite::Error::Error(int code) noexcept
    : msg(sqlite3_errstr(code))
{}

SQLite::Error::Error(sqlite3 *db) noexcept
    : msg(sqlite3_errmsg(db))
{}

SQLite::Error::Error(const char *s) noexcept
    : msg(s)
{}

SQLite::Error::~Error() = default;

auto SQLite::Error::what() const noexcept -> const char * {
    return msg.c_str();
}
