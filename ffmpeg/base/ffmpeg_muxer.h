#ifndef FFMPEG_MUXER_
#define FFMPEG_MUXER_

#include <iostream>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
}

class FFmpegMuxer {
public:
    FFmpegMuxer();
    ~FFmpegMuxer();

public:
    void file_muxer(const std::string& filename);

private:
    AVFormatContext* avformat_out_context_;
    AVCodecContext* video_context_;
};

#endif // ! FFMPEG_MUXER_
