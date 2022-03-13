#include "tagfs.hxx"

TagFS::TagFS(int argc, char **argv, std::filesystem::path datadir)
    : Fuse(argc, argv)
    , db((datadir / ".yatagfs.db").c_str())
{}
