#include "ffmpeg_filter.h"

#include "fmt/format.h"

FFmpegFilter::FFmpegFilter()
    : avformat_context_(nullptr)
    , video_context_(nullptr)
    , buffer_context_(nullptr)
    , buffersink_context_(nullptr)
    , filter_graph_(nullptr)
    , video_index_(-1) {}

FFmpegFilter::~FFmpegFilter() {
    if (avformat_context_) {
        avformat_free_context(avformat_context_);
    }
}

void FFmpegFilter::open_filter_file(const std::string& filename, const std::string& filter_desc) {
    if (filename.empty()) {
        av_log(NULL, AV_LOG_ERROR, "filename is null\n");
        return;
    }

    int ret = -1;

    ret = avformat_open_input(&avformat_context_, filename.c_str(), NULL, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_open_input failed\n");
        return;
    }

    avformat_find_stream_info(avformat_context_, NULL);
    // av_dump_format(avformat_context_, -1, filename.c_str(), 0);

    // 视频
    video_index_ = av_find_best_stream(avformat_context_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (video_index_ == -1) {
        av_log(NULL, AV_LOG_ERROR, "av_find_best_stream video failed\n");
        return;
    }

    AVStream* video_stream = avformat_context_->streams[video_index_];
    const AVCodec* decoder = avcodec_find_decoder(video_stream->codecpar->codec_id);
    if (!decoder) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_find_decoder video failed\n");
        return;
    }

    video_context_ = avcodec_alloc_context3(decoder);
    if (!video_context_) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 video failed\n");
        return;
    }

    ret = avcodec_parameters_to_context(video_context_, video_stream->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_to_context failed\n");
        return;
    }

    ret = avcodec_open2(video_context_, NULL, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_open2 video failed\n");
        return;
    }

    add_video_filter(video_context_, video_stream, filter_desc);
}

void FFmpegFilter::add_video_filter(const AVCodecContext* video_context, const AVStream* video_stream,
                                    const std::string& filter_desc) {
    if (!video_context || !video_stream) {
        av_log(NULL, AV_LOG_ERROR, "add_video_filter video_context or video_stream is null\n");
        return;
    }

    if (filter_desc.empty()) {
        av_log(NULL, AV_LOG_ERROR, "add_video_filter filter_desc is null\n");
        return;
    }

    int ret = -1;
    const AVFilter* buffer_filter = avfilter_get_by_name("buffer");
    const AVFilter* buffersink_filter = avfilter_get_by_name("buffersink");
    AVFilterInOut* inputs = avfilter_inout_alloc();
    AVFilterInOut* outputs = avfilter_inout_alloc();
    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};

    // 滤镜图
    filter_graph_ = avfilter_graph_alloc();
    if (!filter_graph_) {
        av_log(NULL, AV_LOG_ERROR, "avfilter_graph_alloc failed\n");
        return;
    }

    // 滤镜信息
    std::string filter_graph_desc = fmt::format(
        "video_size={}x{}:pix_fmt={}:time_base={}/{}:pixel_aspect={}/{}", video_context->width, video_context->height,
        (int)video_context->pix_fmt, video_stream->time_base.num, video_stream->time_base.den,
        video_stream->sample_aspect_ratio.num, video_stream->sample_aspect_ratio.den);

    // 创建滤镜实例
    ret = avfilter_graph_create_filter(&buffer_context_, buffer_filter, "in", filter_graph_desc.c_str(), NULL,
                                       filter_graph_);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avfilter_graph_create_filter in failed\n");
        return;
    }

    ret = avfilter_graph_create_filter(&buffersink_context_, buffersink_filter, "out", NULL, NULL, filter_graph_);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avfilter_graph_create_filter in failed\n");
        return;
    }

    // 给输出滤镜设置像素格式
    ret = av_opt_set_int_list(buffersink_context_, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "av_opt_set_int_list failed\n");
        return;
    }

    // 设置滤镜的输入、输出参数
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffer_context_;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = "out";
    inputs->filter_ctx = buffersink_context_;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    // 添加过滤字符到滤镜图
    ret = avfilter_graph_parse_ptr(filter_graph_, filter_desc.c_str(), &inputs, &outputs, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avfilter_graph_parse_ptr failed\n");
        return;
    }

    // 检查过滤字符串的有效性, 配置滤镜图中前后连接和图像格式
    ret = avfilter_graph_config(filter_graph_, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avfilter_graph_config failed\n");
        return;
    }

    // 释放输入、输出参数
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
}