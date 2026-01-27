#ifndef YUYVDECODER_H
#define YUYVDECODER_H

#include <cstdint>
#include <vector>

namespace uvc2gl {

    class YuyvDecoder {
        public:
            YuyvDecoder() = default;
            ~YuyvDecoder() = default;

            YuyvDecoder(const YuyvDecoder&) = delete;
            YuyvDecoder& operator=(const YuyvDecoder&) = delete;

            // Convert YUYV to RGB
            // YUYV is 4:2:2 format: Y0 U Y1 V (2 pixels in 4 bytes)
            bool DecodeToRGB(const uint8_t* yuyvData, int width, int height, std::vector<uint8_t>& out);
    };

} // namespace uvc2gl

#endif // YUYVDECODER_H
