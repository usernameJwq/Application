#include <iostream>

#include "gtest/gtest.h"

#include "../rtaudio_player.h"

class RtAudioPlayerTest : public ::testing::Test {
protected:
    void SetUp() override { rtaudio_player_ = std::make_unique<RtAudioPlayer>(); }

    std::unique_ptr<RtAudioPlayer> rtaudio_player_;
};

TEST_F(RtAudioPlayerTest, rtaudio_player_) {
    std::string audiofile = "F:/Qt_Cpp/road123.mp4";
    //    rtaudio_player_->audio_decodec_->load_audio_file(audiofile);
    //    rtaudio_player_->run();
    //
    //    while (true) {
    //        auto state = rtaudio_player_->cur_player_state();
    //        if (rtaudio_player_->cur_player_state() == PlayerStatus::playstop) {
    //            exit(0);
    //        }
    //    }
}