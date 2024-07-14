#ifndef FFMPEG_CODEC_
#define FFMPEG_CODEC_

#include <iostream>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
}

class FFmpegDemuxer {
public:
    FFmpegDemuxer();
    ~FFmpegDemuxer();

public:
    void file_demuxer(const std::string& filename);
    void extract_yuv(const std::string& filename);

private:
    void open_file(const std::string& filename);
    void open_decoder_context(const AVFormatContext* avformat_context, const AVStream* stream);
    void decoder_video(AVCodecContext* video_context, const AVPacket* packet);
    void save_yuv(const AVFrame* frame);

private:
    AVFormatContext* avformat_context_;
    AVCodecContext* video_context_;
    AVCodecContext* audio_context_;

    int video_index_;
    int audio_index_;
};

#endif // !FFMPEG_CODEC_
