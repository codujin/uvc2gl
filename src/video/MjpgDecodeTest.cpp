extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <fstream>
#include <iostream>
#include <vector>

int main() {
    const char* filename = "frame_004.jpg";

    av_log_set_level(AV_LOG_ERROR);

    AVFormatContext* fmt = nullptr;
    if (avformat_open_input(&fmt, filename, nullptr, nullptr) < 0) {
        std::cerr << "Failed to open file\n";
        return 1;
    }

    if (avformat_find_stream_info(fmt, nullptr) < 0) {
        std::cerr << "Failed to find stream info\n";
        return 1;
    }

    const AVCodec* codec = nullptr;
    int streamIndex = av_find_best_stream(fmt, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (streamIndex < 0) {
        std::cerr << "No video stream\n";
        return 1;
    }

    AVCodecContext* ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(ctx, fmt->streams[streamIndex]->codecpar);

    if (avcodec_open2(ctx, codec, nullptr) < 0) {
        std::cerr << "Failed to open decoder\n";
        return 1;
    }

    AVPacket pkt;
    AVFrame* frame = av_frame_alloc();
    AVFrame* rgb = av_frame_alloc();

    int w = ctx->width;
    int h = ctx->height;

    int rgbSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, w, h, 1);
    std::vector<uint8_t> rgbBuffer(rgbSize);

    av_image_fill_arrays(rgb->data, rgb->linesize,
                          rgbBuffer.data(),
                          AV_PIX_FMT_RGB24,
                          w, h, 1);

    SwsContext* sws = sws_getContext(
        w, h, ctx->pix_fmt,
        w, h, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    while (av_read_frame(fmt, &pkt) >= 0) {
        if (pkt.stream_index == streamIndex) {
            avcodec_send_packet(ctx, &pkt);
            if (avcodec_receive_frame(ctx, frame) == 0) {
                sws_scale(sws,
                          frame->data, frame->linesize,
                          0, h,
                          rgb->data, rgb->linesize);
                break;
            }
        }
        av_packet_unref(&pkt);
    }

    // Save RGB as PPM
    std::ofstream out("out.ppm", std::ios::binary);
    out << "P6\n" << w << " " << h << "\n255\n";
    out.write(reinterpret_cast<char*>(rgbBuffer.data()), rgbSize);
    out.close();

    std::cout << "Wrote out.ppm (" << w << "x" << h << ")\n";

    return 0;
}
