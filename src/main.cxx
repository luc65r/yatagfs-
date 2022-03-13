#include <filesystem>
#include <iostream>
#include <optional>

#include "tagfs.hxx"

struct args {
    std::optional<std::filesystem::path> datadir;
};

enum {
    KEY_VERSION,
    KEY_HELP,
};

#define TAGFS_OPT(t, p, v) { t, offsetof(struct tagfs, p), v }
static const struct fuse_opt tagfs_opts[] = {
    FUSE_OPT_KEY("-V", KEY_VERSION),
    FUSE_OPT_KEY("--version", KEY_VERSION),
    FUSE_OPT_KEY("-h", KEY_HELP),
    FUSE_OPT_KEY("--help", KEY_HELP),
    FUSE_OPT_END
};

static void tagfs_usage(struct fuse_args *args) {
    printf("usage: %s datadir mountpoint [options]\n"
           "\n"
           "    -h   --help      print help\n"
           "    -V   --version   print version\n"
           "    -o opt,[opt...]  mount options\n"
           "\n"
           "FUSE options:\n",
           args->argv[0]);
    fuse_lib_help(args);
}

static int tagfs_opt_proc(
    void *data,
    const char *arg,
    int key,
    struct fuse_args *outargs
) {
    struct args *tagfs = static_cast<struct args *>(data);

    switch (key) {
    case FUSE_OPT_KEY_NONOPT:
        if (!tagfs->datadir) {
            tagfs->datadir = std::filesystem::canonical(arg);
            return 0;
        }
        return 1;

    case KEY_VERSION:
        printf("YATAGFS version 0.1.0\n"
               "FUSE library version %s\n",
               fuse_pkgversion());
        fuse_lowlevel_version();
        exit(0);

    case KEY_HELP:
        tagfs_usage(outargs);
        exit(1);
    }

    return 1;
}

auto main(int argc, char **argv) -> int {
    Fuse::Args args(argc, argv);
    struct args tagfs{};
    args.parse(&tagfs, tagfs_opts, tagfs_opt_proc);

    if (!tagfs.datadir) {
        tagfs_usage(&args.args);
        return 1;
    }

    TagFS fuse(args.args.argc, args.args.argv, std::move(*tagfs.datadir));
    fuse.run();

    return 0;
}
