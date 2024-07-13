#include "ffmpeg_demuxer.h"

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
    open_decoder_context(avformat_context_, video_stream);

    // 音频
    audio_index_ = av_find_best_stream(avformat_context_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audio_index_ == -1) {
        av_log(NULL, AV_LOG_ERROR, "av_find_best_stream audio failed\n");
        return;
    }

    AVStream* audio_stream = avformat_context_->streams[audio_index_];
    open_decoder_context(avformat_context_, audio_stream);
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