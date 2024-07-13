#ifndef FFMPEG_FILTER_
#define FFMPEG_FILTER_

#include <iostream>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
}

class FFmpegFilter {
public:
    FFmpegFilter();
    ~FFmpegFilter();

public:
    void open_filter_file(const std::string& filename, const std::string& filter_desc);
    void add_video_filter(const AVCodecContext* video_context, const AVStream* video_stream,
                          const std::string& filter_desc);

private:
    AVFormatContext* avformat_context_;
    AVCodecContext* video_context_;

    AVFilterContext* buffer_context_;
    AVFilterContext* buffersink_context_;
    AVFilterGraph* filter_graph_;

    int video_index_;
};

#endif // !FFMPEG_FILTER_
