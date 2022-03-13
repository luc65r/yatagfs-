#include <filesystem>

#include "fuse.hxx"
#include "sqlite.hxx"

class TagFS : public Fuse {
    SQLite db;
    int datadirfd;

public:
    TagFS(int argc, char **argv, std::filesystem::path datadir);
};
