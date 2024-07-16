#include "ffmpeg_demuxer.h"

#include <fstream>

#include "fmt/format.h"

FFmpegDemuxer::FFmpegDemuxer()
    : avformat_context_(nullptr)
    , video_context_(nullptr)
    , audio_context_(nullptr)
    , video_index_(-1)
    , audio_index_(-1) {}

FFmpegDemuxer::~FFmpegDemuxer() {
    if (avformat_context_) {
        avformat_free_context(avformat_context_);
    }
    if (video_context_) {
        avcodec_free_context(&video_context_);
    }
    if (audio_context_) {
        avcodec_free_context(&audio_context_);
    }
}

void FFmpegDemuxer::file_demuxer(const std::string& filename) {
    open_file(filename);
    open_decoder_context(avformat_context_, avformat_context_->streams[video_index_]);
    open_decoder_context(avformat_context_, avformat_context_->streams[audio_index_]);
}

void FFmpegDemuxer::extract_yuv(const std::string& filename) {
    open_file(filename);
    open_decoder_context(avformat_context_, avformat_context_->streams[video_index_]);

    AVPacket* packet = av_packet_alloc();
    while (av_read_frame(avformat_context_, packet) >= 0) {
        if (packet->stream_index == video_index_) {
            decoder_video(video_context_, packet);
        }
        av_packet_unref(packet);
    }
    av_packet_free(&packet);
}

void FFmpegDemuxer::generate_jpeg(const std::string& filename) {
    open_file(filename);
    open_decoder_context(avformat_context_, avformat_context_->streams[video_index_]);

    int jpeg_index = 1;
    AVPacket* packet = av_packet_alloc();

    while (av_read_frame(avformat_context_, packet) >= 0) {
        if (packet->stream_index == video_index_) {
            int ret = -1;

            ret = avcodec_send_packet(video_context_, packet);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }

            if (ret < 0) {
                break;
            }

            AVFrame* frame = av_frame_alloc();
            while (ret == 0) {
                ret = avcodec_receive_frame(video_context_, frame);
                if (ret == 0) {
                    save_jpeg(frame, jpeg_index);
                    jpeg_index++;
                }
                av_frame_unref(frame);
            }
            if (frame) {
                av_frame_free(&frame);
            }
        }
        av_packet_unref(packet);
    }
    if (packet) {
        av_packet_free(&packet);
    }
}

void FFmpegDemuxer::open_file(const std::string& filename) {
    if (filename.empty()) {
        av_log(NULL, AV_LOG_ERROR, "demuxer filename is null\n");
        return;
    }

    int ret = -1;

    ret = avformat_open_input(&avformat_context_, filename.c_str(), NULL, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_open_input failed\n");
        return;
    }

    ret = avformat_find_stream_info(avformat_context_, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_find_stream_info failed\n");
        return;
    }

    // 打印文件格式信息
    // av_dump_format(avformat_context_, -1, filename.c_str(), 0);

    const AVInputFormat* avinput_format = avformat_context_->iformat;

    // 视频
    video_index_ = av_find_best_stream(avformat_context_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (video_index_ == -1) {
        av_log(NULL, AV_LOG_ERROR, "av_find_best_stream video failed\n");
        return;
    }

    AVStream* video_stream = avformat_context_->streams[video_index_];

    // 音频
    audio_index_ = av_find_best_stream(avformat_context_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audio_index_ == -1) {
        av_log(NULL, AV_LOG_ERROR, "av_find_best_stream audio failed\n");
        return;
    }

    AVStream* audio_stream = avformat_context_->streams[audio_index_];
}

void FFmpegDemuxer::open_decoder_context(const AVFormatContext* avformat_context, const AVStream* stream) {
    if (!avformat_context || !stream) {
        av_log(NULL, AV_LOG_ERROR, "open_decoder_context avformat_context or stream is null\n");
        return;
    }

    int ret = -1;
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);

    if (stream->index == video_index_) {
        video_context_ = avcodec_alloc_context3(decoder);
        if (!video_context_) {
            av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 video failed\n");
            return;
        }

        ret = avcodec_parameters_to_context(video_context_, stream->codecpar);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_to_context video failed\n");
            return;
        }

        ret = avcodec_open2(video_context_, NULL, NULL);
        if (ret != 0) {
            av_log(NULL, AV_LOG_ERROR, "avcodec_open2 video failed\n");
            return;
        }
        av_log(NULL, AV_LOG_INFO, "avcodec_open2 video success\n");

    } else if (stream->index == audio_index_) {
        audio_context_ = avcodec_alloc_context3(decoder);
        if (!audio_context_) {
            av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 audio failed\n");
            return;
        }

        ret = avcodec_parameters_to_context(audio_context_, stream->codecpar);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_to_context audio failed\n");
            return;
        }

        ret = avcodec_open2(audio_context_, NULL, NULL);
        if (ret != 0) {
            av_log(NULL, AV_LOG_ERROR, "avcodec_open2 audio failed\n");
            return;
        }
        av_log(NULL, AV_LOG_INFO, "avcodec_open2 audio success\n");
    }
}

void FFmpegDemuxer::decoder_video(AVCodecContext* video_context, const AVPacket* packet) {
    int ret = -1;

    ret = avcodec_send_packet(video_context, packet);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        return;
    }

    AVFrame* frame = av_frame_alloc();
    while (true) {
        ret = avcodec_receive_frame(video_context, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }
        save_yuv(frame);
    }

    av_frame_free(&frame);
}

void FFmpegDemuxer::save_yuv(const AVFrame* frame) {
    if (!frame) {
        return;
    }

    // 一帧一帧的写入需要追加写入, yuv文件很大
    std::ofstream ofs("out.yuv", std::ios::out | std::ios::binary | std::ios::app);
    if (ofs.is_open()) {
        for (int i = 0; i < frame->height; i++) {
            ofs.write((const char*)(frame->data[0] + frame->linesize[0] * i), frame->width);
        }
        for (int i = 0; i < frame->height / 2; i++) {
            ofs.write((const char*)(frame->data[1] + frame->linesize[1] * i), frame->width / 2);
        }
        for (int i = 0; i < frame->height / 2; i++) {
            ofs.write((const char*)(frame->data[2] + frame->linesize[2] * i), frame->width / 2);
        }
    }
    ofs.flush();
    ofs.close();
}

void FFmpegDemuxer::save_jpeg(const AVFrame* frame, const int& jpeg_index) {
    if (jpeg_index % 300 != 0) {
        return;
    }

    if (!frame) {
        av_log(NULL, AV_LOG_ERROR, "save_jpeg frame is null\n");
        return;
    }

    // 像素格式转换
    SwsContext* sws_context = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format, frame->width,
                                             frame->height, AV_PIX_FMT_YUVJ420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);

    AVFrame* j420p_frame = av_frame_alloc();
    j420p_frame->format = frame->format;
    j420p_frame->width = frame->width;
    j420p_frame->height = frame->height;

    // 申请转换后的frame存储空间
    int size =
        av_image_alloc(j420p_frame->data, j420p_frame->linesize, frame->width, frame->height, AV_PIX_FMT_YUVJ420P, 1);
    if (size <= 0) {
        av_log(NULL, AV_LOG_ERROR, "av_image_alloc failed\n");
        return;
    }

    int scale_height = sws_scale(sws_context, frame->data, frame->linesize, 0, frame->height, j420p_frame->data,
                                 j420p_frame->linesize);
    if (scale_height <= 0) {
        av_log(NULL, AV_LOG_ERROR, "sws_scale failed\n");
        return;
    }

    if (sws_context) {
        sws_freeContext(sws_context);
    }

    int ret = 0;
    AVFormatContext* jpeg_format_context = nullptr;

    std::string jpeg_name = fmt::format("jpeg_test/road_{}.jpeg", jpeg_index);

    ret = avformat_alloc_output_context2(&jpeg_format_context, NULL, NULL, jpeg_name.c_str());
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_alloc_output_context2 failed\n");
        return;
    }

    // 打开编码器
    const AVCodec* encoder = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if (!encoder) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_find_encoder AV_CODEC_ID_MJPEG failed\n");
        return;
    }

    AVCodecContext* jpeg_codec_context = avcodec_alloc_context3(encoder);
    if (!jpeg_codec_context) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 failed\n");
        return;
    }
    jpeg_codec_context->width = frame->width;
    jpeg_codec_context->height = frame->height;
    jpeg_codec_context->pix_fmt = AV_PIX_FMT_YUVJ420P;
    jpeg_codec_context->time_base = {1, 25};

    ret = avcodec_open2(jpeg_codec_context, NULL, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_open2 failed\n");
        return;
    }

    AVStream* video_stream = avformat_new_stream(jpeg_format_context, NULL);
    if (!video_stream) {
        av_log(NULL, AV_LOG_ERROR, "avformat_new_stream failed\n");
        return;
    }

    ret = avformat_write_header(jpeg_format_context, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_write_header failed\n");
        return;
    }

    AVPacket* packet = av_packet_alloc();
    while (avcodec_send_frame(jpeg_codec_context, j420p_frame) >= 0) {
        ret = avcodec_receive_packet(jpeg_codec_context, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }

        if (ret < 0) {
            break;
        }

        av_write_frame(jpeg_format_context, packet);
        av_frame_unref(j420p_frame);
        av_packet_unref(packet);
    }

    if (j420p_frame) {
        av_frame_free(&j420p_frame);
    }

    if (packet) {
        av_packet_free(&packet);
    }

    av_write_trailer(jpeg_format_context);

    if (jpeg_format_context) {
        avformat_free_context(jpeg_format_context);
    }

    if (jpeg_codec_context) {
        avcodec_close(jpeg_codec_context);
        avcodec_free_context(&jpeg_codec_context);
    }
}