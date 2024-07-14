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

TEST_F(FFmpegMuxerTest, copy_file) {
    std::string src_file = "F:/Qt_Cpp/road123.mp4";
    std::string des_file = "F:/Qt_Cpp/roadcopy.mp4";
    // ffmpeg_muxer_->copy_file(src_file, des_file);
}
