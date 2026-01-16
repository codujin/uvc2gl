#include <cstddef>
#include <ios>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

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
struct Buffer {
    void* start = nullptr;
    size_t length = 0;
};

int main (int argc, char **argv){
    std::string dev = (argc >= 2) ? argv[1] : "/dev/video1"; // default device
    int frameCount = (argc >=3) ? std::atoi(argv[2]) : 60; // default frame count
    
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
    // Request buffers
    v4l2_requestbuffers reqBuffer{};
    reqBuffer.count = 4; // Request 4 buffers
    reqBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqBuffer.memory = V4L2_MEMORY_MMAP;

    if (xioctl(fd, VIDIOC_REQBUFS, &reqBuffer) < 0) {
        std::cerr << "Error requesting buffers: " << strerror(errno) << std::endl;
        close(fd);
        return 1;
    }

    std::vector<Buffer> buffers(reqBuffer.count);

    for (size_t i = 0; i < reqBuffer.count; ++i){
        v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (xioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
            std::cerr << "Error querying buffer " << i << ": " << strerror(errno) << std::endl;
            close(fd);
            return 1;
        }

        buffers[i].length = buf.length;
        buffers[i].start = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (buffers[i].start == MAP_FAILED) {
            std::cerr << "Error mapping buffer " << i << ": " << strerror(errno) << std::endl;
            close(fd);
            return 1;
        }
    }

    // Queue buffers
    for (size_t i = 0; i < reqBuffer.count; ++i){
        v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (xioctl(fd, VIDIOC_QBUF, &buf) < 0) {
            std::cerr << "Error queueing buffer " << i << ": " << strerror(errno) << std::endl;
            close(fd);
            return 1;
        }
    }

    // Start streaming
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        std::cerr << "Error starting streaming: " << strerror(errno) << std::endl;
        close(fd);
        return 1;
    }

    // Capture frames
    std::cout << "Capturing " << frameCount << " frames..." << std::endl;
    for (int frame = 0; frame <frameCount; ++frame){
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        timeval tv{};
        tv.tv_sec = 2; // 2 seconds timeout
        tv.tv_usec = 0;

        int r = select(fd + 1, &fds, nullptr, nullptr, &tv);
        if (r < 0) {
            std::cerr << "Error in select: " << strerror(errno) << std::endl;
            break;
        }
        if (r == 0){
            std::cerr << "Timeout waiting for frame" << std::endl;
            break;
        }

        v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (xioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
            std::cerr << "Error dequeueing buffer: " << strerror(errno) << std::endl;
            break;
        }

        // process frame (just get its size for now)
        const void *data = buffers[buf.index].start;
        size_t length = buf.bytesused;

        // write to file
        std::ostringstream filename;
        filename << "frame_" << std::setfill('0') << std::setw(3) << frame << ".jpg";
        std::ofstream ofs(filename.str(), std::ios::binary);
        ofs.write(reinterpret_cast<const char *>(data), static_cast<std::streamsize>(length));
        ofs.close();

        if (frame % 10 == 0)
            std::cout << "Captured frame " << frame << ", size: " << length << " bytes" << std::endl;

        // Re-queue buffer
        if (xioctl(fd, VIDIOC_QBUF, &buf) < 0) {
            std::cerr << "Error re-queueing buffer: " << strerror(errno) << std::endl;
            break;
        }
    }

    // Stop streaming
    if (xioctl(fd, VIDIOC_STREAMOFF, &type) < 0) {
        std::cerr << "Error stopping streaming: " << strerror(errno) << std::endl;
    }

    // Unmap buffers
    for (size_t i = 0; i < reqBuffer.count; ++i){
        munmap(buffers[i].start, buffers[i].length);
    }
    close(fd);
    std::cout << "Capture complete." << std::endl;
    return 0;
}