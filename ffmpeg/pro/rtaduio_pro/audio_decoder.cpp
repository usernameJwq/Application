#include "audio_decoder.h"

#include <thread>
#include <fstream>

AudioDecoder::AudioDecoder()
    : avformat_context_(nullptr)
    , audio_context_(nullptr)
    , audio_stream_(nullptr)
    , player_state_(PlayerStatus::playnoting) {}

AudioDecoder::~AudioDecoder() {
    if (avformat_context_) {
        avformat_close_input(&avformat_context_);
        avformat_free_context(avformat_context_);
    }
    if (audio_context_) {
        avcodec_free_context(&audio_context_);
    }
}

void AudioDecoder::load_audio_file(const std::string& audiofile) {
    if (open_audio_file(audiofile)) {
        std::thread(&AudioDecoder::audio_decode, this).detach();
    }
}

void AudioDecoder::generate_audio_pcm() {
    std::ofstream ofs("test_pcm.pcm", std::ios::binary | std::ios::out);
    if (ofs.is_open()) {
        while (!audio_queue_.empty()) {
            auto audio_frame = audio_queue_.front();
            audio_queue_.pop();
            ofs.write(audio_frame.frame_data, audio_frame.frame_length);
            free(audio_frame.frame_data);
        }
    }
    ofs.flush();
    ofs.close();
}

bool AudioDecoder::open_audio_file(const std::string& audiofile) {
    if (audiofile.empty()) {
        av_log(NULL, AV_LOG_ERROR, "audiofile is empty\n");
        return false;
    }

    int ret = -1;
    ret = avformat_open_input(&avformat_context_, audiofile.c_str(), NULL, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_open_input failed\n");
        return false;
    }

    ret = avformat_find_stream_info(avformat_context_, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_find_stream_info failed\n");
        return false;
    }

    int audio_index = av_find_best_stream(avformat_context_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audio_index < 0) {
        av_log(NULL, AV_LOG_ERROR, "av_find_best_stream failed\n");
        return false;
    }

    audio_stream_ = avformat_context_->streams[audio_index];
    if (!audio_stream_) {
        av_log(NULL, AV_LOG_ERROR, "audio_stream_ is null\n");
        return false;
    }

    const AVCodec* decoder = avcodec_find_decoder(audio_stream_->codecpar->codec_id);
    if (!decoder) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_find_decoder is null\n");
        return false;
    }

    audio_context_ = avcodec_alloc_context3(decoder);
    if (!audio_context_) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 failed\n");
        return false;
    }

    ret = avcodec_parameters_to_context(audio_context_, audio_stream_->codecpar);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_to_context failed\n");
        return false;
    }

    ret = avcodec_open2(audio_context_, NULL, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_open2 failed\n");
        return false;
    }
    return true;
}

void AudioDecoder::audio_decode() {
    int ret = 0;
    AVPacket* packet = av_packet_alloc();

    while (av_read_frame(avformat_context_, packet) >= 0) {
        if (packet->stream_index == audio_stream_->index) {
            if (avcodec_send_packet(audio_context_, packet) == 0) {
                AVFrame* frame = av_frame_alloc();
                while (true) {
                    ret = avcodec_receive_frame(audio_context_, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;
                    }

                    if (ret == 0) {
                        audio_frame_resample(frame, 2, 44100, AV_SAMPLE_FMT_S16);
                    }
                    av_frame_unref(frame);
                }
                av_frame_free(&frame);
            }
        }
        av_packet_unref(packet);
    }
    av_packet_free(&packet);
}

void AudioDecoder::audio_frame_resample(const AVFrame* src_frame, const int& out_channel, const int& out_samplerate,
                                        AVSampleFormat sample_format) {
    int ret = -1;

    SwrContext* swr_context = swr_alloc();
    swr_context = swr_alloc_set_opts(swr_context, av_get_default_channel_layout(out_channel), sample_format,
                                     out_samplerate, src_frame->channel_layout, (AVSampleFormat)src_frame->format,
                                     src_frame->sample_rate, 0, NULL);
    if (!swr_context) {
        av_log(NULL, AV_LOG_ERROR, "swr_alloc_set_opts failed\n");
        return;
    }
    ret = swr_init(swr_context);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "swr_init failed\n");
        return;
    }

    // 获取采样之间的延时
    int sample_delay = swr_get_delay(swr_context, audio_stream_->codecpar->sample_rate) + src_frame->nb_samples;

    // 根据 采样大小/采样频率==音频时长 进行转换
    int dst_sample = av_rescale_rnd(sample_delay, out_samplerate, audio_stream_->codecpar->sample_rate, AVRounding(1));

    unsigned char* buffer[2] = {0};
    buffer[0] = (unsigned char*)calloc(dst_sample * out_channel * 16 / 8, 1);
    int sample_size =
        swr_convert(swr_context, buffer, dst_sample, (const uint8_t**)src_frame->data, src_frame->nb_samples);
    if (sample_size <= 0) {
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    push_audio_data((char*)buffer[0], sample_size * out_channel * 16 / 8);
    free(buffer[0]);

    if (swr_context) {
        swr_close(swr_context);
        swr_free(&swr_context);
    }
}

void AudioDecoder::push_audio_data(const char* data, const int& length) {
    if (!data || length <= 0) {
        return;
    }
    if (player_state_ != PlayerStatus::playaudio) {
        player_state_ = PlayerStatus::playaudio;
    }
    audio_queue_.push(AudioData((char*)data, length));
}

void AudioDecoder::set_rtaudio_out_frame(char* outputBuffer) {
    if (!audio_queue_.empty()) {
        auto frame = audio_queue_.front();
        audio_queue_.pop();

        int out_channel = 2;
        int seek_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

        // 位深为16bit, 左右通道数据就是两个字节, 先左通道, 再右通道
        for (int channel = 0; channel < out_channel; channel++) {
            for (int i = 0; i < frame.frame_length / 2; i++) {
                outputBuffer[seek_size * i + 2 * channel] = frame.frame_data[2 * i];
                outputBuffer[seek_size * i + 2 * channel + 1] = frame.frame_data[2 * i + 1];
            }
        }
        if (frame.frame_data) {
            free(frame.frame_data);
        }
    } else {
        player_state_ = PlayerStatus::playstop;
    }
}

PlayerStatus AudioDecoder::cur_play_state() { return player_state_; }