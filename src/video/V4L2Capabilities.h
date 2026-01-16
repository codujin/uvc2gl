#ifndef V4L2CAPABILITIES_H
#define V4L2CAPABILITIES_H

#include <string>
#include <vector>

namespace UVC2GL {

struct VideoFormat {
    int width;
    int height;
    int fps;
    
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
};

class V4L2Capabilities {
public:
    static std::vector<VideoFormat> QueryFormats(const std::string& device);
    static std::vector<VideoDevice> EnumerateDevices();
};

}

#endif // V4L2CAPABILITIES_H
