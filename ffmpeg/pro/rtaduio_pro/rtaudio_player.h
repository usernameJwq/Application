#ifndef RTAUDIO_PLAY_
#define RTAUDIO_PLAY_

#include <iostream>

#include "rtaudio/RtAudio.h"

#include "audio_decoder.h"

class RtAudioPlayer {
public:
    RtAudioPlayer();
    ~RtAudioPlayer();

public:
    void run();
    PlayerStatus cur_player_state();

private:
    static int out(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime,
                   RtAudioStreamStatus status, void *userData);

private:
    void init_rtaudio_player();
    bool asio_open();

public:
    std::shared_ptr<AudioDecoder> audio_decodec_;

private:
    static RtAudioPlayer *player_;
    std::shared_ptr<RtAudio> rt_audio_;
    RtAudio::Api api_;
    static PlayerStatus player_state_;
};

#endif // !RTAUDIO_PLAY_
