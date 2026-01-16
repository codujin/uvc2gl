#ifndef MJPGDECODER_H
#define MJPGDECODER_H

#include <cstdint>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}


namespace uvc2gl {

    class MjpgDecoder{
        public:
            MjpgDecoder();
            ~MjpgDecoder();

            MjpgDecoder(const MjpgDecoder&) = delete;
            MjpgDecoder& operator=(const MjpgDecoder&) = delete;

            bool DecodeToRGB(const unsigned char* mjpgData, size_t mjpgSize, int& width, int& height, std::vector<uint8_t>& out);
        private:
            AVCodecContext* m_codecCtx;
            AVFrame* m_frame;
            AVPacket* m_packet;
            SwsContext* m_swsCtx;

            int m_width;
            int m_height;
            AVPixelFormat m_pixFmt = AV_PIX_FMT_NONE;
            void ResetSwsContext(int width, int height, AVPixelFormat pixFmt);
    };
}

#endif // MJPGDECODER_H