#ifndef FFMPEG_CODEC_
#define FFMPEG_CODEC_

#include <iostream>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
}

class FFmpegCodec {
public:
    FFmpegCodec();
    ~FFmpegCodec();

public:
    void file_decode(std::string& filename);

private:
    AVFormatContext* avformat_context_;
    int video_index_;
    int audio_index_;
};

#endif // !FFMPEG_CODEC_
