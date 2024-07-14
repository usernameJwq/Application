#include "ffmpeg_demuxer.h"

#include <fstream>

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
