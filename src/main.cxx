#include <iostream>

#include "tagfs.hxx"

auto main(int argc, char **argv) -> int {
    TagFS fuse(argc, argv);
    fuse.run();
    return 0;
}
