#include "V4L2Capabilities.h"
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
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

}
