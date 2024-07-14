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
    void copy_file(const std::string& src_file, const std::string& des_file);

private:
    void open_file(const std::string& filename);
    void copy_av_stream(const AVStream* video_stream, const AVStream* audio_stream, const std::string& des_file);

private:
    AVFormatContext* avformat_context_;
    AVFormatContext* avformat_out_context_;

    AVCodecContext* video_context_;

    AVStream* video_stream_;
    AVStream* audio_stream_;

    int video_index_;
    int audio_index_;
};

#endif // ! FFMPEG_MUXER_
