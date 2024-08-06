#ifndef FFMPEG_FILTER_
#define FFMPEG_FILTER_

#include <iostream>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersrc.h"
#include "libavfilter/buffersink.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
}

class FFmpegFilter {
public:
    FFmpegFilter();
    ~FFmpegFilter();

public:
    void add_video_filter(const std::string& input_filename, const std::string& output_filename,
                          const std::string& filter_desc);

private:
    void open_input_file(const std::string& filename);
    void init_encoder();
    void init_filter(const std::string& filter_desc);
    void init_out_context(const std::string& output_filename);

private:
    void decodec();
    void write_video_buffer(AVPacket* packet);
    void video_frame_src_to_sink(AVFrame* frame, AVPacket* src_packet);
    void recoder_video(AVFrame* frame, AVPacket* src_packet);

private:
    AVFrame* video_scale(AVFrame* frame);

private:
    AVFormatContext* avformat_context_;
    AVFormatContext* avformat_out_context_;

    AVCodecContext* video_context_;
    AVCodecContext* audio_context_;
    AVCodecContext* video_encoder_context_;
    AVCodecContext* audio_encoder_context_;

    AVStream* video_stream_;
    AVStream* audio_stream_;

    AVFilterContext* buffer_context_;
    AVFilterContext* buffersink_context_;
    AVFilterGraph* filter_graph_;
};

#endif // !FFMPEG_FILTER_
