#define FUSE_USE_VERSION 35
#include <fuse.h>
#include <fuse_lowlevel.h>

class Fuse {
    struct fuse *fuse;
    struct fuse_args args;
    struct fuse_operations op;
    struct fuse_cmdline_opts opts;

public:
    Fuse(int argc, char **argv);
    virtual ~Fuse();

    Fuse(const Fuse &) = delete;
    Fuse &operator=(const Fuse &) = delete;

    auto run() -> int;

    virtual auto getattr(const char *, struct stat *, struct fuse_file_info *fi) -> int;
};
