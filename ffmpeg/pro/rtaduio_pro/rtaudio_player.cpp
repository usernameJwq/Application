#include "rtaudio_player.h"

#include <vector>
#include <thread>

#include "boost/algorithm/string.hpp"

RtAudioPlayer *RtAudioPlayer::player_ = nullptr;
PlayerStatus RtAudioPlayer::player_state_ = PlayerStatus::playnoting;

int RtAudioPlayer::out(void *outputBuffer, void *inputBuffer, unsigned int nFrames, double streamTime,
                       RtAudioStreamStatus status, void *userData) {
    if (streamTime < 2) {
        memset((char *)outputBuffer, 0, 1024);
        return 0;
    }

    if (player_->audio_decodec_->cur_play_state() == PlayerStatus::playaudio) {
        if (player_state_ != PlayerStatus::playaudio) {
            player_state_ = PlayerStatus::playaudio;
        }
        player_->audio_decodec_->set_rtaudio_out_frame((char *)outputBuffer);

    } else if (player_->audio_decodec_->cur_play_state() == PlayerStatus::playstop) {
        if (player_->rt_audio_->isStreamRunning()) {
            player_state_ = PlayerStatus::playstop;
            player_->rt_audio_->stopStream();
        }
    }
    return 0;
}

RtAudioPlayer::RtAudioPlayer() {
    player_ = this;
    audio_decodec_ = std::make_shared<AudioDecoder>();
}

RtAudioPlayer::~RtAudioPlayer() {}

void RtAudioPlayer::run() {
    std::thread([=] {
        init_rtaudio_player();
        if (asio_open()) {
            if (rt_audio_->isStreamOpen() && !rt_audio_->isStreamRunning()) {
                rt_audio_->startStream();
            }
        }
    }).detach();
}

PlayerStatus RtAudioPlayer::cur_player_state() { return player_state_; }

void RtAudioPlayer::init_rtaudio_player() {
    api_ = RtAudio::Api::WINDOWS_ASIO;
    rt_audio_ = std::make_shared<RtAudio>();
    if (rt_audio_ == nullptr) {
        api_ = RtAudio::Api::WINDOWS_WASAPI;
        rt_audio_ = std::make_shared<RtAudio>(api_);
    }
}

bool RtAudioPlayer::asio_open() {
    RtAudio::StreamParameters ostream_parmeters;
    ostream_parmeters.deviceId = rt_audio_->getDefaultOutputDevice();
    ostream_parmeters.nChannels = 2;
    unsigned int sampleRate = 44100;
    unsigned int bufferFrames = 1024;

    rt_audio_->openStream(&ostream_parmeters, NULL, RTAUDIO_SINT16, sampleRate, &bufferFrames, out);
    if (!rt_audio_->isStreamOpen()) {
        std::cout << "openStream failed" << std::endl;
        return false;
    }
    return true;
}
