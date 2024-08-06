#include "ffmpeg_filter.h"

#include <thread>

#include "fmt/format.h"

FFmpegFilter::FFmpegFilter()
    : avformat_context_(nullptr)
    , avformat_out_context_(nullptr)
    , video_context_(nullptr)
    , audio_context_(nullptr)
    , video_encoder_context_(nullptr)
    , audio_encoder_context_(nullptr)
    , video_stream_(nullptr)
    , audio_stream_(nullptr)
    , buffer_context_(nullptr)
    , buffersink_context_(nullptr)
    , filter_graph_(nullptr) {}

FFmpegFilter::~FFmpegFilter() {
    if (avformat_context_) {
        avformat_free_context(avformat_context_);
    }
}

void FFmpegFilter::add_video_filter(const std::string& input_filename, const std::string& output_filename,
                                    const std::string& filter_desc) {
    if (input_filename.empty() || output_filename.empty()) {
        av_log(NULL, AV_LOG_ERROR, "input_filename or output_filename is null\n");
        return;
    }

    if (filter_desc.empty()) {
        av_log(NULL, AV_LOG_ERROR, "filter_desc is null\n");
        return;
    }

    open_input_file(input_filename);
    init_filter(filter_desc);
    init_out_context(output_filename);
    std::thread(&FFmpegFilter::decodec, this).join();

    // 写文件尾，关闭输出流
    recoder_video(NULL, NULL);
    write_video_buffer(NULL);
    av_write_trailer(avformat_out_context_);
    avio_close(avformat_out_context_->pb);
    av_log(NULL, AV_LOG_INFO, "write_video_and_audio_stream over !\n");
}

void FFmpegFilter::open_input_file(const std::string& filename) {
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
    int video_index = av_find_best_stream(avformat_context_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (video_index < 0) {
        av_log(NULL, AV_LOG_ERROR, "av_find_best_stream video failed\n");
        return;
    }

    video_stream_ = avformat_context_->streams[video_index];
    const AVCodec* decoder = avcodec_find_decoder(video_stream_->codecpar->codec_id);
    if (!decoder) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_find_decoder video failed\n");
        return;
    }

    video_context_ = avcodec_alloc_context3(decoder);
    if (!video_context_) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 video failed\n");
        return;
    }

    ret = avcodec_parameters_to_context(video_context_, video_stream_->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_to_context failed\n");
        return;
    }

    ret = avcodec_open2(video_context_, NULL, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_open2 video failed\n");
        return;
    }

    // 音频
    int audio_index = av_find_best_stream(avformat_context_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audio_index < 0) {
        av_log(NULL, AV_LOG_ERROR, "av_find_best_stream audio failed\n");
        return;
    }

    audio_stream_ = avformat_context_->streams[audio_index];
    const AVCodec* aud_decoder = avcodec_find_decoder(audio_stream_->codecpar->codec_id);
    if (!aud_decoder) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_find_decoder audio failed\n");
        return;
    }

    audio_context_ = avcodec_alloc_context3(aud_decoder);
    if (!audio_context_) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 audio failed\n");
        return;
    }

    ret = avcodec_parameters_to_context(audio_context_, audio_stream_->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_to_context audio failed\n");
        return;
    }

    ret = avcodec_open2(audio_context_, NULL, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_open2 audio failed\n");
        return;
    }
}

void FFmpegFilter::init_encoder() {
    int ret = -1;

    // 视频
    const AVCodec* vid_encoder = avcodec_find_encoder(video_stream_->codecpar->codec_id);
    if (!vid_encoder) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_find_encoder failed\n");
        return;
    }

    video_encoder_context_ = avcodec_alloc_context3(vid_encoder);
    if (!video_encoder_context_) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 video failed\n");
        return;
    }

    video_encoder_context_->codec_id = video_stream_->codecpar->codec_id;
    video_encoder_context_->width = av_buffersink_get_w(buffersink_context_);
    video_encoder_context_->height = av_buffersink_get_h(buffersink_context_);
    video_encoder_context_->framerate = av_buffersink_get_frame_rate(buffersink_context_);
    video_encoder_context_->time_base = {1, 25};
    video_encoder_context_->framerate = {25, 1};
    video_encoder_context_->gop_size = video_context_->gop_size;
    video_encoder_context_->pix_fmt = (AVPixelFormat)av_buffersink_get_format(buffersink_context_);

    ret = avcodec_open2(video_encoder_context_, NULL, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_open2 video failed\n");
        return;
    }

    // 音频
    const AVCodec* aud_encoder = avcodec_find_encoder(audio_stream_->codecpar->codec_id);
    if (!aud_encoder) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_find_encoder audio failed\n");
        return;
    }

    audio_encoder_context_ = avcodec_alloc_context3(aud_encoder);
    if (!audio_encoder_context_) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 audio failed\n");
        return;
    }

    ret = avcodec_parameters_to_context(audio_encoder_context_, audio_stream_->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_to_context audio failed\n");
        return;
    }

    ret = avcodec_open2(audio_encoder_context_, NULL, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_open2 audio failed\n");
        return;
    }
}

void FFmpegFilter::init_filter(const std::string& filter_desc) {
    if (filter_desc.empty()) {
        av_log(NULL, AV_LOG_ERROR, "add_video_filter filter_desc is null\n");
        return;
    }

    int ret = -1;
    const AVFilter* buffersrc = avfilter_get_by_name("buffer");
    const AVFilter* buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut* inputs = avfilter_inout_alloc();
    AVFilterInOut* outputs = avfilter_inout_alloc();
    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUYV422, AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE};

    // 滤镜图
    filter_graph_ = avfilter_graph_alloc();
    if (!filter_graph_) {
        av_log(NULL, AV_LOG_ERROR, "avfilter_graph_alloc failed\n");
        return;
    }

    // 滤镜信息
    std::string filter_graph_desc = fmt::format(
        "video_size={}x{}:pix_fmt={}:time_base={}/{}:pixel_aspect={}/{}", video_context_->width, video_context_->height,
        (int)video_context_->pix_fmt, video_stream_->time_base.num, video_stream_->time_base.den,
        video_stream_->sample_aspect_ratio.num, video_stream_->sample_aspect_ratio.den);

    // 创建滤镜实例
    ret =
        avfilter_graph_create_filter(&buffer_context_, buffersrc, "in", filter_graph_desc.c_str(), NULL, filter_graph_);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avfilter_graph_create_filter in failed\n");
        return;
    }

    ret = avfilter_graph_create_filter(&buffersink_context_, buffersink, "out", NULL, NULL, filter_graph_);
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
    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_context_;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffer_context_;
    outputs->pad_idx = 0;
    outputs->next = NULL;

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

void FFmpegFilter::init_out_context(const std::string& output_filename) {
    int ret = -1;
    ret = avformat_alloc_output_context2(&avformat_out_context_, NULL, NULL, output_filename.c_str());
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_alloc_output_context2 failed\n");
        return;
    }

    ret = avio_open(&avformat_out_context_->pb, output_filename.c_str(), AVIO_FLAG_READ_WRITE);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avio_open failed\n");
        return;
    }

    init_encoder();

    // 创建数据流
    AVStream* video_stream = avformat_new_stream(avformat_out_context_, NULL);
    ret = avcodec_parameters_copy(video_stream->codecpar, video_stream_->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "video stream avcodec_parameters_copy failed\n");
        return;
    }
    video_stream->time_base = video_stream_->time_base;
    video_stream->codecpar->codec_tag = 0;

    AVStream* audio_stream = avformat_new_stream(avformat_out_context_, NULL);
    ret = avcodec_parameters_copy(audio_stream->codecpar, audio_stream_->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "video stream avcodec_parameters_copy failed\n");
        return;
    }
    audio_stream->time_base = audio_stream_->time_base;
    audio_stream->codecpar->codec_tag = 0;

    ret = avformat_write_header(avformat_out_context_, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_write_header failed\n");
        return;
    }
}

void FFmpegFilter::decodec() {
    int ret = -1;
    AVPacket* packet = av_packet_alloc();
    while (av_read_frame(avformat_context_, packet) == 0) {
        if (packet->stream_index == video_stream_->index) {
            write_video_buffer(packet);
        } else if (packet->stream_index == audio_stream_->index) {
            packet->stream_index = 1;
            av_write_frame(avformat_out_context_, packet);
        }
        av_packet_unref(packet);
    }

    if (packet) {
        av_packet_free(&packet);
    }
}

void FFmpegFilter::write_video_buffer(AVPacket* packet) {
    int ret = -1;

    ret = avcodec_send_packet(video_context_, packet);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return;
    }

    if (ret < 0) {
        return;
    }

    if (ret == 0) {
        AVFrame* frame = av_frame_alloc();
        while (true) {
            ret = avcodec_receive_frame(video_context_, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }

            if (ret < 0) {
                break;
            }

            if (ret == 0) {
                video_frame_src_to_sink(frame, packet);
            }
            av_frame_unref(frame);
        }
        av_frame_free(&frame);
    }
}

void FFmpegFilter::video_frame_src_to_sink(AVFrame* frame, AVPacket* src_packet) {
    if (!frame) {
        return;
    }

    int ret = -1;
    ret = av_buffersrc_add_frame_flags(buffer_context_, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "av_buffersrc_add_frame_flags failed\n");
        return;
    }

    AVFrame* fliter_frame = av_frame_alloc();
    while (true) {
        ret = av_buffersink_get_frame(buffersink_context_, fliter_frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }

        if (ret < 0) {
            break;
        }

        recoder_video(fliter_frame, src_packet);
        av_frame_unref(fliter_frame);
    }

    if (fliter_frame) {
        av_frame_free(&fliter_frame);
    }
}

static int pts_ = 0;

void FFmpegFilter::recoder_video(AVFrame* frame, AVPacket* src_packet) {
    int ret = -1;
    AVPacket* packet = av_packet_alloc();

    if (frame != NULL) {
        frame = video_scale(frame);
    }

    ret = avcodec_send_frame(video_encoder_context_, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return;
    }

    if (ret < 0) {
        return;
    }

    while (true) {
        ret = avcodec_receive_packet(video_encoder_context_, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }

        if (ret < 0) {
            break;
        }

        av_packet_rescale_ts(packet, video_context_->time_base, video_encoder_context_->time_base);
        if (src_packet != NULL) {
            packet->pts = src_packet->pts;
            packet->dts = src_packet->dts;
        }
        packet->stream_index = 0;
        av_write_frame(avformat_out_context_, packet);
    }
    if (packet) {
        av_packet_free(&packet);
    }
}

AVFrame* FFmpegFilter::video_scale(AVFrame* frame) {
    if (frame == NULL) {
        return NULL;
    }

    SwsContext* sws_context = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format,
                                             video_encoder_context_->width, video_encoder_context_->height,
                                             video_encoder_context_->pix_fmt, SWS_FAST_BILINEAR, NULL, NULL, NULL);

    AVFrame* sws_frame = av_frame_alloc();
    sws_frame->width = frame->width;
    sws_frame->height = frame->height;
    sws_frame->format = frame->format;
    sws_frame->pict_type = frame->pict_type;
    sws_frame->pts = frame->pts;
    sws_frame->pkt_dts = frame->pkt_dts;

    int alloc_size = av_image_alloc(sws_frame->data, sws_frame->linesize, sws_frame->width, sws_frame->height,
                                    (AVPixelFormat)sws_frame->format, 1);
    if (alloc_size <= 0) {
        av_frame_free(&sws_frame);
        return NULL;
    }

    int out_height =
        sws_scale(sws_context, frame->data, frame->linesize, 0, frame->height, sws_frame->data, sws_frame->linesize);

    if (sws_context) {
        sws_freeContext(sws_context);
    }

    if (out_height > 0) {
        return sws_frame;
    }
    return NULL;
}