#include "YuyvDecoder.h"
#include <iostream>
#include <vector>
#include <cstdint>

using namespace uvc2gl;

int main() {
    // Create a simple test pattern: 2x2 YUYV image
    // YUYV format: Y0 U Y1 V (4 bytes for 2 pixels)
    std::vector<uint8_t> yuyvData = {
        // Row 1: 2 pixels
        128, 128, 128, 128,  // Gray pixel 1 & 2
        // Row 2: 2 pixels
        255, 128, 255, 128   // White pixel 1 & 2
    };

    YuyvDecoder decoder;
    std::vector<uint8_t> rgbData;

    int width = 2;
    int height = 2;

    std::cout << "Testing YUYV decoder with " << width << "x" << height << " image..." << std::endl;
    
    if (decoder.DecodeToRGB(yuyvData.data(), width, height, rgbData)) {
        std::cout << "Decode successful!" << std::endl;
        std::cout << "Output RGB data size: " << rgbData.size() << " bytes" << std::endl;
        std::cout << "Expected: " << (width * height * 3) << " bytes" << std::endl;
        
        // Print RGB values
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 3;
                std::cout << "Pixel[" << x << "," << y << "]: "
                         << "R=" << (int)rgbData[idx] << " "
                         << "G=" << (int)rgbData[idx+1] << " "
                         << "B=" << (int)rgbData[idx+2] << std::endl;
            }
        }
        
        return 0;
    } else {
        std::cerr << "Decode failed!" << std::endl;
        return 1;
    }
}
