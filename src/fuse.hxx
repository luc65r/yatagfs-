#define FUSE_USE_VERSION 35
#include <fuse.h>
#include <fuse_lowlevel.h>

class Fuse {
public:
    class Args {
    public:
        struct fuse_args args;

        Args(int argc, char **argv);
        ~Args();

        auto parse(void *data, const struct fuse_opt *, fuse_opt_proc_t) -> void;
        auto parse_cmdline(struct fuse_cmdline_opts *) -> void;
    };

private:
    Args args;
    struct fuse *fuse;
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
