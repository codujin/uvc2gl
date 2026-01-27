#ifndef V4L2CAPABILITIES_H
#define V4L2CAPABILITIES_H

#include <cstdint>
#include <string>
#include <vector>

namespace uvc2gl {

struct VideoFormat {
    int width;
    int height;
    int fps;
    uint32_t pixelFormat;  // V4L2 pixel format (e.g., V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_YUYV)
    
    std::string toString() const {
        return std::to_string(width) + "x" + std::to_string(height) + " @ " + std::to_string(fps) + "fps";
    }
};

struct VideoDevice {
    std::string path;           // e.g., "/dev/video0"
    std::string name;           // Device name from capabilities
    std::string driver;         // Driver name
    
    std::string toString() const {
        return name + " (" + path + ")";
    }
    
    std::string displayName() const {
        return name;
    }
};

class V4L2Capabilities {
public:
    static std::vector<VideoFormat> QueryFormats(const std::string& device);
    static std::vector<VideoDevice> EnumerateDevices();
};

}

#endif // V4L2CAPABILITIES_H
