#include "ffmpeg_muxer.h"

FFmpegMuxer::FFmpegMuxer()
    : avformat_context_(nullptr)
    , avformat_out_context_(nullptr)
    , video_context_(nullptr)
    , video_stream_(nullptr)
    , audio_stream_(nullptr)
    , video_index_(-1)
    , audio_index_(-1) {}

FFmpegMuxer::~FFmpegMuxer() {
    if (avformat_context_) {
        avformat_close_input(&avformat_context_);
        avformat_free_context(avformat_context_);
    }
    if (avformat_out_context_) {
        avformat_free_context(avformat_out_context_);
    }
    if (video_context_) {
        avcodec_free_context(&video_context_);
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

void FFmpegMuxer::copy_file(const std::string& src_file, const std::string& des_file) {
    open_file(src_file);
    copy_av_stream(video_stream_, audio_stream_, des_file);
}

void FFmpegMuxer::open_file(const std::string& filename) {
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

    ret = avformat_find_stream_info(avformat_context_, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_find_stream_info failed\n");
        return;
    }

    // 视频
    video_index_ = av_find_best_stream(avformat_context_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (video_index_ == -1) {
        av_log(NULL, AV_LOG_ERROR, "av_find_best_stream video failed\n");
        return;
    }
    video_stream_ = avformat_context_->streams[video_index_];

    // 音频
    audio_index_ = av_find_best_stream(avformat_context_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audio_index_ == -1) {
        av_log(NULL, AV_LOG_ERROR, "av_find_best_stream audio failed\n");
        return;
    }
    audio_stream_ = avformat_context_->streams[audio_index_];
}

/*
 * 剥离视频和音频同理, 只创建一个数据流即可
 *
 * 截取视频：
 *  时间戳 = 时间点 x 频率
 *  可以使用 av_seek_frame() 和 av_q2d() 进行实现
 */
void FFmpegMuxer::copy_av_stream(const AVStream* video_stream, const AVStream* audio_stream,
                                 const std::string& des_file) {
    int ret = -1;

    // 分配封装实例
    ret = avformat_alloc_output_context2(&avformat_out_context_, NULL, NULL, des_file.c_str());
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_alloc_output_context2 failed\n");
        return;
    }

    // 打开输出流
    ret = avio_open(&avformat_out_context_->pb, des_file.c_str(), AVIO_FLAG_READ_WRITE);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avio_open failed\n");
        return;
    }

    // 新建数据流
    AVStream* new_vido_stream = avformat_new_stream(avformat_out_context_, NULL);
    ret = avcodec_parameters_copy(new_vido_stream->codecpar, video_stream->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_copy video failed\n");
        return;
    }
    new_vido_stream->time_base = video_stream->time_base;
    new_vido_stream->codecpar->codec_tag = 0;

    AVStream* new_audio_stream = avformat_new_stream(avformat_out_context_, NULL);
    ret = avcodec_parameters_copy(new_audio_stream->codecpar, audio_stream->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_copy audio failed\n");
        return;
    }
    new_audio_stream->time_base = audio_stream->time_base;
    new_audio_stream->codecpar->codec_tag = 0;

    // 写文件头
    ret = avformat_write_header(avformat_out_context_, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_write_header failed\n");
        return;
    }

    // 写入文件
    AVPacket* packet = av_packet_alloc();
    while (av_read_frame(avformat_context_, packet) >= 0) {
        if (packet->stream_index == video_stream->index) {
            packet->stream_index = 0;
        } else if (packet->stream_index == audio_stream->index) {
            packet->stream_index = 1;
        }
        av_write_frame(avformat_out_context_, packet);
        av_packet_unref(packet);
    }

    // 写文件尾
    ret = av_write_trailer(avformat_out_context_);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "av_write_trailer failed\n");
        return;
    }

    av_packet_free(&packet);

    // 关闭文件输出流
    avio_close(avformat_out_context_->pb);
}