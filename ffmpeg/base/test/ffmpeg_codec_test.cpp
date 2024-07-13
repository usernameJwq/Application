#include <iostream>

#include "gtest/gtest.h"

#include "../ffmpeg_codec.h"

class FFmpegCodecTest : public ::testing::Test {
protected:
    void SetUp() override { ffmpeg_codec_ = std::make_unique<FFmpegCodec>(); }

    std::unique_ptr<FFmpegCodec> ffmpeg_codec_;
};

TEST_F(FFmpegCodecTest, file_decode) {
    std::string filename = "F:/Qt_Cpp/cmake_pro/Application/assets/video/road.mp4";
    ffmpeg_codec_->file_decode(filename);
}
