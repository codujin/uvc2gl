#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <cstring>


//ioctl wrapper that restarts if interrupted by signal
static int xioctl(int fd, unsigned long request, void *arg) {
    int r;
    do {
        r = ioctl(fd, request, arg);
    } while (r == -1 && errno == EINTR);
    return r;
}

// Convert V4L2 FourCC format to string
static std::string v4l2FourCCToString(__u32 fmt){
    char fourcc[5] = {
        static_cast<char>(fmt & 0xFF),
        static_cast<char>((fmt >> 8) & 0xFF),
        static_cast<char>((fmt >> 16) & 0xFF),
        static_cast<char>((fmt >> 24) & 0xFF),
        '\0'
    };
    return std::string(fourcc);
}

int main (int argc, char **argv){
    std::string dev = (argc >= 2) ? argv[1] : "/dev/video1"; // default device

    int fd = open(dev.c_str(), O_RDWR | O_CLOEXEC); // Open device
    if (fd < 0) { 
        std::cerr << "Error opening device " << dev << ": " << strerror(errno) << std::endl;
        return 1;
    }

    // Query device capabilities
    v4l2_capability caps{}; 
    if (xioctl(fd, VIDIOC_QUERYCAP, &caps) < 0) {
        std::cerr << "Error querying capabilities: " << strerror(errno) << std::endl;
        close(fd);
        return 1;
    }

    std::cout << "Device: " << dev << std::endl;
    std::cout << "Driver: " << caps.driver << std::endl;
    std::cout << "Card: " << caps.card << std::endl;
    std::cout << "Bus info: " << caps.bus_info << std::endl;
    std::cout << "Caps: " << std::hex << caps.capabilities << std::dec << std::endl;

    // Query supported formats

    v4l2_format fmt{};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (xioctl(fd, VIDIOC_G_FMT, &fmt) < 0) {
        std::cerr << "Error getting format: " << strerror(errno) << std::endl;
        close(fd);
        return 1;
    }
    std::cout << "Current Format: "
        << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height
        << " fourcc: " << v4l2FourCCToString(fmt.fmt.pix.pixelformat)
        << " bytesperline: " << fmt.fmt.pix.bytesperline
        << " sizeimage: " << fmt.fmt.pix.sizeimage
        << std::endl;

    // Set format for 1080p

    v4l2_format fmt1080p{};
    fmt1080p.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt1080p.fmt.pix.width = 1920;
    fmt1080p.fmt.pix.height = 1080;
    fmt1080p.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG; // MJPEG format
    fmt1080p.fmt.pix.field = V4L2_FIELD_ANY;

    if (xioctl(fd, VIDIOC_S_FMT, &fmt1080p) < 0) {
        std::cerr << "Error setting format: " << strerror(errno) << std::endl;
        close(fd);
        return 1;
    }

    std::cout << "Negotiated Format: "
        << fmt1080p.fmt.pix.width << "x" << fmt1080p.fmt.pix.height
        << " fourcc: " << v4l2FourCCToString(fmt1080p.fmt.pix.pixelformat)
        << " bytesperline: " << fmt1080p.fmt.pix.bytesperline
        << " sizeimage: " << fmt1080p.fmt.pix.sizeimage
        << std::endl;
    close(fd);
    return 0;

}