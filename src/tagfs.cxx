#include "tagfs.hxx"

TagFS::TagFS(int argc, char **argv, std::filesystem::path datadir)
    : Fuse(argc, argv)
    , db((datadir / ".yatagfs.db").c_str())
{
    db.exec("pragma recursive_triggers = true");

    db.exec(R"(
create table if not exists files
    ( id integer primary key not null
    , path text not null unique
    );

create table if not exists tags
    ( id integer primary key not null
    , name text not null unique
    );

create table if not exists files_tags
    ( file_id integer not null
    , tag_id integer not null
    , primary key (file_id, tag_id)
    , foreign key (file_id) references files (id) on delete cascade
    , foreign key (tag_id) references tags (id) on delete cascade
    );
)");
}
