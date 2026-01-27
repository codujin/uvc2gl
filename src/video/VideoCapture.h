#ifndef VIDEO_CAPTURE_H
#define VIDEO_CAPTURE_H

#include "Frame.h"
#include "MjpgDecoder.h"
#include "YuyvDecoder.h"
#include "RingBuffer.h"
#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <thread>

namespace uvc2gl {
    class VideoCapture {
        public:
            VideoCapture(std::string device, int width, int height, int fps, std::string format, size_t ringBufferSize);
            ~VideoCapture();

            VideoCapture(const VideoCapture&) = delete;
            VideoCapture& operator=(const VideoCapture&) = delete;

            void Start();
            void Stop();
            bool IsRunning() const { return m_Running.load(); }

            std::optional<Frame> GetFrame();

        private:
            void CaptureLoop();
            std::string m_Device;
            int m_Width;
            int m_Height;
            int m_FPS;
            std::string m_Format;

            std::unique_ptr<RingBuffer> m_RingBuffer;
            std::unique_ptr<MjpgDecoder> m_mjpegDecoder;
            std::unique_ptr<YuyvDecoder> m_yuyvDecoder;
            std::thread m_CaptureThread;
            std::atomic<bool> m_Running;
    };
}

#endif // VIDEO_CAPTURE_H