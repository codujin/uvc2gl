#ifndef FRAME_H
#define FRAME_H

#include <cstdint>
#include <vector>
namespace UVC2GL{
struct Frame {
    int width;
    int height;
    std::vector<uint8_t> data;
    uint64_t timestamp; // in nanoseconds

};
}

#endif // FRAME_H