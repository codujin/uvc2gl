#include "V4L2Capabilities.h"
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <iostream>

namespace UVC2GL {

static int xioctl(int fd, unsigned long request, void *arg) {
    int r;
    do {
        r = ioctl(fd, request, arg);
    } while (r == -1 && errno == EINTR);
    return r;
}

std::vector<VideoFormat> V4L2Capabilities::QueryFormats(const std::string& device) {
    std::vector<VideoFormat> formats;
    
    int fd = open(device.c_str(), O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        std::cerr << "Failed to open device: " << device << std::endl;
        return formats;
    }
    
    // Query MJPEG format sizes
    v4l2_frmsizeenum frmsize{};
    frmsize.pixel_format = V4L2_PIX_FMT_MJPEG;
    frmsize.index = 0;
    
    while (xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
        if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
            int width = frmsize.discrete.width;
            int height = frmsize.discrete.height;
            
            // Query framerates for this resolution
            v4l2_frmivalenum frmival{};
            frmival.pixel_format = V4L2_PIX_FMT_MJPEG;
            frmival.width = width;
            frmival.height = height;
            frmival.index = 0;
            
            while (xioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0) {
                if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
                    int fps = frmival.discrete.denominator / frmival.discrete.numerator;
                    formats.push_back({width, height, fps});
                }
                frmival.index++;
            }
        }
        frmsize.index++;
    }
    
    close(fd);
    return formats;
}

std::vector<VideoDevice> V4L2Capabilities::EnumerateDevices() {
    std::vector<VideoDevice> devices;
    
    DIR* dir = opendir("/dev");
    if (!dir) {
        std::cerr << "Failed to open /dev directory" << std::endl;
        return devices;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Look for video* devices
        if (strncmp(entry->d_name, "video", 5) == 0) {
            std::string path = "/dev/" + std::string(entry->d_name);
            
            int fd = open(path.c_str(), O_RDWR | O_CLOEXEC);
            if (fd < 0) {
                continue; // Skip devices we can't open
            }
            
            v4l2_capability caps{};
            if (xioctl(fd, VIDIOC_QUERYCAP, &caps) >= 0) {
                // Check if it's a video capture device
                if (caps.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
                    VideoDevice device;
                    device.path = path;
                    device.name = reinterpret_cast<const char*>(caps.card);
                    device.driver = reinterpret_cast<const char*>(caps.driver);
                    devices.push_back(device);
                }
            }
            
            close(fd);
        }
    }
    
    closedir(dir);
    return devices;
}

}
