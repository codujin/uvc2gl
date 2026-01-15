#ifndef VIDEO_CAPTURE_H
#define VIDEO_CAPTURE_H

#include "Frame.h"
#include "MjpgDecoder.h"
#include "RingBuffer.h"
#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <thread>

namespace UVC2GL {
    class VideoCapture {
        public:
            VideoCapture(std::string device, int width, int height, int fps, size_t ringBufferSize);
            ~VideoCapture();

            VideoCapture(const VideoCapture&) = delete;
            VideoCapture& operator=(const VideoCapture&) = delete;

            void Start();
            void Stop();

            std::optional<Frame> GetFrame();

        private:
            void CaptureLoop();
            std::string m_Device;
            int m_Width;
            int m_Height;
            int m_FPS;

            std::unique_ptr<RingBuffer> m_RingBuffer;
            std::unique_ptr<MjpgDecoder> m_decoder;
            std::thread m_CaptureThread;
            std::atomic<bool> m_Running;
    };
}

#endif // VIDEO_CAPTURE_H