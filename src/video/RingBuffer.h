#ifndef RINGBUFFER_H
#define RINGBUFFER_H
#include "Frame.h"
#include <cstddef>
#include <mutex>
#include <optional>

namespace UVC2GL {
    class RingBuffer {
        public:
            explicit RingBuffer(size_t capacity)
                : m_Capacity(capacity), m_FrameBuffer(capacity) {}
            void push(Frame&& frame){
                std::lock_guard<std::mutex> lock(m_Mutex);
                m_FrameBuffer[m_Write] = std::move(frame);
                m_Write = (m_Write + 1) % m_Capacity;
            }

            std::optional<Frame> pop(){
                std::lock_guard<std::mutex> lock(m_Mutex);
                if (m_Count == 0) {
                    return std::nullopt; // Buffer is empty
                }
                size_t newIdx = (m_Write + m_Capacity - 1) % m_Capacity;
                auto frame = std::move(m_FrameBuffer[newIdx]);
                m_Count = 0;
                return frame;
            }
        private:
            size_t m_Capacity;
            std::vector<std::optional<Frame>> m_FrameBuffer;
            size_t m_Write = 0;
            size_t m_Count = 0;
            std::mutex m_Mutex;
    };
}

#endif // RINGBUFFER_H