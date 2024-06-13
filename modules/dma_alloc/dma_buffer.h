#pragma  once

#include <ios>

using DMABuffer = struct {
    int fd;
    size_t size;
    void *buffer;
};