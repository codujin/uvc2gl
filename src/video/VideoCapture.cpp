#include "VideoCapture.h"
#include "Frame.h"

#include <bits/types/struct_timeval.h>
#include <cstdint>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace UVC2GL {
    //ioctl wrapper that restarts if interrupted by signal
    static int xioctl(int fd, unsigned long request, void *arg) {
        int r;
        do {
            r = ioctl(fd, request, arg);
        } while (r == -1 && errno == EINTR);
        return r;
    }


    struct Buffer {
        void* start = nullptr;
        size_t length = 0;
    };

    VideoCapture::VideoCapture(std::string device, int width, int height, int fps, size_t ringBufferSize)
        : m_Device(std::move(device)), m_Width(width), m_Height(height), m_FPS(fps) {
        m_RingBuffer = std::make_unique<RingBuffer>(ringBufferSize);
        m_decoder = std::make_unique<MjpgDecoder>();
        m_Running = false;
    }

    VideoCapture::~VideoCapture() {
        Stop();
    }

    void VideoCapture::Start() {
        if (m_Running.exchange(true))
            return; // already running
        m_CaptureThread = std::thread(&VideoCapture::CaptureLoop, this);
    }

    void VideoCapture::Stop(){
        if (!m_Running.exchange(false))
            return; // not running
        if (m_CaptureThread.joinable())
            m_CaptureThread.join();
    }

    std::optional<Frame> VideoCapture::GetFrame() {
        return m_RingBuffer->pop();
    }

    void VideoCapture::CaptureLoop(){
        try {
            int fd = open(m_Device.c_str(), O_RDWR | O_CLOEXEC); // Open device
            if (fd < 0)
                throw std::runtime_error("Error opening device " + m_Device + ": " + strerror(errno));
        // set format

        v4l2_format fmt{};
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = m_Width;
        fmt.fmt.pix.height = m_Height;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
        fmt.fmt.pix.field = V4L2_FIELD_ANY;
        if (xioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
            close(fd);
            throw std::runtime_error("Error setting format: " + std::string(strerror(errno)));
        }

        v4l2_requestbuffers reqBuffer{};
        reqBuffer.count = 4; // Request 4 buffers
        reqBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        reqBuffer.memory = V4L2_MEMORY_MMAP;
        if (xioctl(fd, VIDIOC_REQBUFS, &reqBuffer) < 0) {
            close(fd);
            throw std::runtime_error("Error requesting buffers: " + std::string(strerror(errno)));
        }

        std::vector<Buffer> buffers(reqBuffer.count);
        for (size_t i = 0; i < reqBuffer.count; ++i){
            v4l2_buffer buff{};
            buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buff.memory = V4L2_MEMORY_MMAP;
            buff.index = i;
            if (xioctl(fd, VIDIOC_QUERYBUF, &buff) < 0) {
                close(fd);
                throw std::runtime_error("Error querying buffer " + std::to_string(i) + ": " + std::string(strerror(errno)));
            }
            buffers[i].length = buff.length;
            buffers[i].start = mmap(nullptr, buff.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buff.m.offset);
            if (buffers[i].start == MAP_FAILED) {
                close(fd);
                throw std::runtime_error("Error mapping buffer " + std::to_string(i) + ": " + std::string(strerror(errno)));
            }
        }

        // Queue buffers
        for (size_t i = 0; i < reqBuffer.count; ++i){
            v4l2_buffer buff{};
            buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buff.memory = V4L2_MEMORY_MMAP;
            buff.index = i;
            if (xioctl(fd, VIDIOC_QBUF, &buff) < 0) {
                close(fd);
                throw std::runtime_error("Error queueing buffer " + std::to_string(i) + ": " + std::string(strerror(errno)));
            }
        }

        // Start streaming
        v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (xioctl(fd, VIDIOC_STREAMON, &type) < 0) {
            close(fd);
            throw std::runtime_error("Error starting streaming: " + std::string(strerror(errno)));
        }

        int warmupFrames = m_FPS; // 1 second worth of frames
        while(m_Running.load()){
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(fd, &fds);
            timeval tv {};
            tv.tv_sec = 2; // 2 second timeout
            tv.tv_usec = 0;

            int r = select(fd + 1, &fds, nullptr, nullptr, &tv);
            if (r <= 0)
                continue;

            v4l2_buffer buff{};
            buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buff.memory = V4L2_MEMORY_MMAP;

            if (xioctl(fd, VIDIOC_DQBUF, &buff) < 0) {
                std::cerr << "Error dequeueing buffer: " << strerror(errno) << std::endl;
                continue;
            }

            const uint8_t* mjpgData = static_cast<uint8_t*>(buffers[buff.index].start);
            size_t mjpgSize = buff.bytesused;

            if (warmupFrames > 0){
                --warmupFrames;
                xioctl(fd, VIDIOC_QBUF, &buff); // re-queue buffer
                continue; // skip processing during warmup
            }

            int width, height;
            std::vector<uint8_t> rgbData;
            if (m_decoder->DecodeToRGB(mjpgData, mjpgSize, width, height, rgbData)){
                Frame frame;
                frame.width = width;
                frame.height = height;
                frame.data = std::move(rgbData);
                m_RingBuffer->push(std::move(frame));
            }
            xioctl(fd, VIDIOC_QBUF, &buff); // re-queue buffer
        }
        xioctl( fd, VIDIOC_STREAMOFF, &type);
        for (size_t i = 0; i < reqBuffer.count; ++i){
            munmap(buffers[i].start, buffers[i].length);
        }
        close(fd);
        } catch (const std::exception& e) {
            std::cerr << "Video capture error: " << e.what() << std::endl;
            m_Running = false;
        }
    }

}