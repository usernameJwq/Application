#include <iostream>

#include "gtest/gtest.h"

#include "../audio_decoder.h"

class AudioDecoderTest : public ::testing::Test {
public:
    void SetUp() override { audio_decoder_ = std::make_unique<AudioDecoder>(); }

    std::unique_ptr<AudioDecoder> audio_decoder_;
};

TEST_F(AudioDecoderTest, audio_decode_test) {
    std::string audiofile = "F:/Qt_Cpp/road123.mp4";
    // audio_decoder_->load_audio_file(audiofile);
    // audio_decoder_->generate_audio_pcm();
}