#include <cerrno>
#include <cstdlib>
#include <stdexcept>

#include "fuse.hxx"

auto Fuse::getattr(
    [[maybe_unused]] const char *path,
    [[maybe_unused]] struct stat *sb,
    [[maybe_unused]] struct fuse_file_info *fi
) -> int {
    return -ENOSYS;
}

static auto do_getattr(
    const char *path,
    struct stat *sb,
    struct fuse_file_info *fi
) noexcept -> int try {
    auto fuse = static_cast<Fuse *>(fuse_get_context()->private_data);
    return fuse->getattr(path, sb, fi);
} catch (...) {
    return -EIO;
}

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
Fuse::Fuse(int argc, char **argv)
    : args(argc, argv)
    , op{
        .getattr = do_getattr,
    }
{
    args.parse_cmdline(&opts);
    if (!opts.mountpoint)
        throw std::runtime_error("no mountpoint specified");

    fuse = fuse_new(&args.args, &op, sizeof op, this);
    if (fuse == NULL)
        throw std::runtime_error("failed to initialize fuse");

    if (fuse_mount(fuse, opts.mountpoint) != 0)
        throw std::runtime_error("failed to mount fuse");

    if (fuse_daemonize(opts.foreground) != 0)
        throw std::runtime_error("failed to daemonize");

    struct fuse_session *se = fuse_get_session(fuse);
    if (fuse_set_signal_handlers(se) != 0)
        throw std::runtime_error("failed to set signal handlers");
}

Fuse::~Fuse() {
    fuse_unmount(fuse);
    fuse_destroy(fuse);
    free(opts.mountpoint);
}

auto Fuse::run() -> int {
    if (opts.singlethread) {
        return fuse_loop(fuse);
    } else {
        struct fuse_loop_config loop_config{
            .clone_fd = opts.clone_fd,
            .max_idle_threads = opts.max_idle_threads,
        };
        return fuse_loop_mt(fuse, &loop_config);
    }
}

Fuse::Args::Args(int argc, char **argv)
    : args{argc, argv, 0}
{}

Fuse::Args::~Args() {
    fuse_opt_free_args(&args);
}

auto Fuse::Args::parse(
    void *data,
    const struct fuse_opt *opts,
    fuse_opt_proc_t proc
) -> void {
    if (fuse_opt_parse(&args, data, opts, proc) != 0)
        throw std::runtime_error("failed to parse args");
}

auto Fuse::Args::parse_cmdline(struct fuse_cmdline_opts *opts) -> void {
    if (fuse_parse_cmdline(&args, opts) != 0)
        throw std::runtime_error("failed to parse cmdline args");
}
