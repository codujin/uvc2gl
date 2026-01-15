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

class V4L2Capabilities {
public:
    static std::vector<VideoFormat> QueryFormats(const std::string& device);
};

}

#endif // V4L2CAPABILITIES_H
