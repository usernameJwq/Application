#include "ffmpeg_codec.h"

FFmpegCodec::FFmpegCodec() : avformat_context_(nullptr), video_index_(-1), audio_index_(-1) {}

FFmpegCodec::~FFmpegCodec() {}

void FFmpegCodec::file_decode(std::string& filename) {
    int ret = -1;
    ret = avformat_open_input(&avformat_context_, filename.c_str(), NULL, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_open_input failed\n");
    }

    ret = avformat_find_stream_info(avformat_context_, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_find_stream_info failed\n");
    }

    av_dump_format(avformat_context_, -1, filename.c_str(), 0);

    video_index_ = av_find_best_stream(avformat_context_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (video_index_ == -1) {
        av_log(NULL, AV_LOG_ERROR, "av_find_best_stream video failed\n");
    }
    av_log(NULL, AV_LOG_DEBUG, "video_index_=%d\n", video_index_);

    AVStream* video_stream = avformat_context_->streams[video_index_];
    const AVCodec* video_decoder = avcodec_find_decoder(video_stream->codecpar->codec_id);
    av_log(NULL, AV_LOG_INFO, "video_decoder id=%d\n", video_decoder->id);
    av_log(NULL, AV_LOG_INFO, "video_decoder name=%s\n", video_decoder->name);
    av_log(NULL, AV_LOG_INFO, "video_decoder long_name=%s\n", video_decoder->long_name);

    audio_index_ = av_find_best_stream(avformat_context_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audio_index_ == -1) {
        av_log(NULL, AV_LOG_ERROR, "av_find_best_stream audio failed\n");
    }
    av_log(NULL, AV_LOG_DEBUG, "audio_index_=%d\n", audio_index_);

    AVStream* audio_stream = avformat_context_->streams[audio_index_];
    const AVCodec* audio_decoder = avcodec_find_decoder(audio_stream->codecpar->codec_id);
    av_log(NULL, AV_LOG_INFO, "audio_decoder id=%d\n", audio_decoder->id);
    av_log(NULL, AV_LOG_INFO, "audio_decoder name=%s\n", audio_decoder->name);
    av_log(NULL, AV_LOG_INFO, "audio_decoder long_name=%s\n", audio_decoder->long_name);
}