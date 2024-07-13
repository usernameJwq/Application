#include <iostream>

#include "gtest/gtest.h"

#include "../ffmpeg_muxer.h"

class FFmpegMuxerTest : public ::testing::Test {
protected:
    void SetUp() override { ffmpeg_muxer_ = std::make_unique<FFmpegMuxer>(); };

    std::unique_ptr<FFmpegMuxer> ffmpeg_muxer_;
};

TEST_F(FFmpegMuxerTest, muxer_file) {
    std::string filename = "out.mp4";
    ffmpeg_muxer_->file_muxer(filename);
}
