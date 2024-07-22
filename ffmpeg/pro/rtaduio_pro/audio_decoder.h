#ifndef AUDIO_DECODER_
#define AUDIO_DECODER_

#include <iostream>
#include <queue>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libavutil/log.h"
}

enum class PlayerStatus { playnoting, playaudio, playstop };

struct AudioData {
    char* frame_data;
    int frame_length;

    AudioData(char* data, const int& length) {
        if (length > 0) {
            frame_data = (char*)calloc(length, 1);
            if (frame_data == nullptr) {
                frame_length = 0;
                return;
            }
            memcpy(frame_data, data, length);
        }
        frame_length = length;
    }
};

class AudioDecoder {
public:
    AudioDecoder();
    ~AudioDecoder();

public:
    void set_rtaudio_out_frame(char* outputBuffer);
    PlayerStatus cur_play_state();

public:
    void load_audio_file(const std::string& audiofile);
    void generate_audio_pcm();

public:
    bool open_audio_file(const std::string& audiofile);

private:
    void audio_decode();
    void audio_frame_resample(const AVFrame* src_frame, const int& out_channel, const int& out_samplerate,
                              AVSampleFormat sample_format);
    void push_audio_data(const char* data, const int& length);

private:
    AVFormatContext* avformat_context_;
    AVCodecContext* audio_context_;
    AVStream* audio_stream_;
    std::queue<AudioData> audio_queue_;
    PlayerStatus player_state_;
};

#endif // !AUDIO_DECODER_
