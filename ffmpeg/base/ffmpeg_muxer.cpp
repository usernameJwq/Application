#include "ffmpeg_muxer.h"

FFmpegMuxer::FFmpegMuxer() : avformat_out_context_(nullptr), video_context_(nullptr) {}

FFmpegMuxer::~FFmpegMuxer() {
    if (avformat_out_context_) {
        avformat_free_context(avformat_out_context_);
    }
}

void FFmpegMuxer::file_muxer(const std::string& filename) {
    if (filename.empty()) {
        av_log(NULL, AV_LOG_ERROR, "muxer filename is null\n");
        return;
    }

    int ret = -1;

    // 分配文件封装实例
    ret = avformat_alloc_output_context2(&avformat_out_context_, NULL, NULL, filename.c_str());
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_alloc_output_context2 failed\n");
        return;
    }

    // 打开输出流
    ret = avio_open(&avformat_out_context_->pb, filename.c_str(), AVIO_FLAG_READ_WRITE);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avio_open failed\n");
        return;
    }

    // 查找编码器创建编码器上下文
    const AVCodec* video_coder = avcodec_find_encoder(AV_CODEC_ID_H264);
    video_context_ = avcodec_alloc_context3(video_coder);
    if (!video_context_) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 video failed\n");
        return;
    }
    video_context_->width = 600;
    video_context_->height = 400;

    // 创建数据流, 复制上下文参数
    AVStream* video_stream = avformat_new_stream(avformat_out_context_, video_coder);
    ret = avcodec_parameters_from_context(video_stream->codecpar, video_context_);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_from_context video failed\n");
        return;
    }
    video_stream->codecpar->codec_tag = 0;

    // 打开文件头, 写入文件数据, 关闭文件尾
    avformat_write_header(avformat_out_context_, NULL);
    av_write_trailer(avformat_out_context_);

    // 关闭数据流
    avio_close(avformat_out_context_->pb);
}