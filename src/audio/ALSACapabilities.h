#pragma once

#include <string>
#include <vector>

namespace uvc2gl {

struct AudioDevice {
    std::string name;        // ALSA device name (e.g., "hw:0,0", "default")
    std::string description; // Human-readable description
    
    std::string toString() const {
        return description.empty() ? name : description;
    }
};

class ALSACapabilities {
public:
    static std::vector<AudioDevice> EnumerateDevices();
};

} // namespace uvc2gl
