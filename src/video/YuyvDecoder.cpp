#include "YuyvDecoder.h"
#include <algorithm>

namespace uvc2gl {

    bool YuyvDecoder::DecodeToRGB(const uint8_t* yuyvData, int width, int height, std::vector<uint8_t>& out) {
        if (!yuyvData || width <= 0 || height <= 0) {
            return false;
        }

        // Allocate output buffer for RGB
        out.resize(width * height * 3);

        // Optimized YUYV to RGB conversion
        // Process 2 pixels at a time (YUYV format: Y0 U Y1 V)
        const int numPixelPairs = (width * height) / 2;
        const uint8_t* src = yuyvData;
        uint8_t* dst = out.data();

        for (int i = 0; i < numPixelPairs; ++i) {
            const int y0 = src[0];
            const int u  = src[1];
            const int y1 = src[2];
            const int v  = src[3];
            src += 4;

            // YUV to RGB conversion (ITU-R BT.601)
            const int c0 = y0 - 16;
            const int c1 = y1 - 16;
            const int d = u - 128;
            const int e = v - 128;

            // First pixel
            int r = (298 * c0 + 409 * e + 128) >> 8;
            int g = (298 * c0 - 100 * d - 208 * e + 128) >> 8;
            int b = (298 * c0 + 516 * d + 128) >> 8;
            dst[0] = std::clamp(r, 0, 255);
            dst[1] = std::clamp(g, 0, 255);
            dst[2] = std::clamp(b, 0, 255);
            dst += 3;

            // Second pixel (reuse d and e)
            r = (298 * c1 + 409 * e + 128) >> 8;
            g = (298 * c1 - 100 * d - 208 * e + 128) >> 8;
            b = (298 * c1 + 516 * d + 128) >> 8;
            dst[0] = std::clamp(r, 0, 255);
            dst[1] = std::clamp(g, 0, 255);
            dst[2] = std::clamp(b, 0, 255);
            dst += 3;
        }

        return true;
    }

} // namespace uvc2gl
