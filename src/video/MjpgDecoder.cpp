#include "MjpgDecoder.h"
#include "libswscale/swscale.h"
#include <cstddef>
#include <stdexcept>
#include <cstring>

extern "C" {
#include <libavutil/imgutils.h>
}

namespace uvc2gl {
    MjpgDecoder::MjpgDecoder() {
        const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
        if (!codec)
            throw std::runtime_error("MJPG decoder not found");
        m_codecCtx = avcodec_alloc_context3(codec);
        if (!m_codecCtx)
            throw std::runtime_error("Failed to allocate codec context");
        if (avcodec_open2(m_codecCtx, codec, nullptr) < 0)
            throw std::runtime_error("Failed to open codec");

        m_frame = av_frame_alloc();
        m_packet = av_packet_alloc();
        m_swsCtx = nullptr;

        if (!m_frame || !m_packet)
            throw std::runtime_error("Failed to allocate frame or packet");
    }

    MjpgDecoder::~MjpgDecoder(){
        if (m_swsCtx)
            sws_freeContext(m_swsCtx);
        if (m_frame)
            av_frame_free(&m_frame);
        if (m_packet)
            av_packet_free(&m_packet);
        if (m_codecCtx)
            avcodec_free_context(&m_codecCtx);
    }

    void MjpgDecoder::ResetSwsContext(int width, int height, AVPixelFormat pixFmt) {
        if (m_swsCtx){
            sws_freeContext(m_swsCtx);
            m_swsCtx = nullptr;
        }

        m_swsCtx = sws_getContext(
            width, height, pixFmt,
            width, height, AV_PIX_FMT_RGB24,
            SWS_BILINEAR, nullptr, nullptr, nullptr);
        if (!m_swsCtx)
            throw std::runtime_error("Failed to create SwsContext");
        m_width = width;
        m_height = height;
        m_pixFmt = pixFmt;
    }

    bool MjpgDecoder::DecodeToRGB(const unsigned char* mjpgData, size_t mjpgSize, int& width, int& height, std::vector<uint8_t>& out) {
        if (mjpgSize < 4)
            return false;
        if (!(mjpgData[0] == 0xFF && mjpgData[1] == 0xD8)) // SOI
            return false;
        if (!(mjpgData[mjpgSize - 2] == 0xFF && mjpgData[mjpgSize - 1] == 0xD9)) // EOI
            return false;

        av_packet_unref(m_packet);
        if (av_new_packet(m_packet, mjpgSize) < 0)
            return false;
        std::memcpy(m_packet->data, mjpgData, mjpgSize);

        if (avcodec_send_packet(m_codecCtx, m_packet) < 0)
            return false;
        int ret = avcodec_receive_frame(m_codecCtx, m_frame);
        if (ret < 0)
            return false;
        width = m_frame->width;
        height = m_frame->height;

        if (!m_swsCtx || width != m_width || height != m_height || m_frame->format != m_pixFmt) {
            ResetSwsContext(width, height, static_cast<AVPixelFormat>(m_frame->format));
        }

        out.resize((size_t)width * (size_t)height * 3);
        uint8_t* destData[4] = { out.data(), nullptr, nullptr, nullptr };
        int destLinesize[4] = { width * 3, 0, 0, 0 };

        sws_scale(m_swsCtx,
                  m_frame->data, m_frame->linesize,
                  0, height,
                  destData, destLinesize);

        return true;
    }
}