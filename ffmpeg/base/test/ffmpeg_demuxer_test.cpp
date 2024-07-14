#include <iostream>

#include "gtest/gtest.h"

#include "../ffmpeg_demuxer.h"

class FFmpegDemuxerTest : public ::testing::Test {
protected:
    void SetUp() override { ffmpeg_demuxer_ = std::make_unique<FFmpegDemuxer>(); }

    std::unique_ptr<FFmpegDemuxer> ffmpeg_demuxer_;
};

TEST_F(FFmpegDemuxerTest, file_demuxer) {
    std::string filename = "F:/Qt_Cpp/cmake_pro/Application/assets/video/road.mp4";
    ffmpeg_demuxer_->file_demuxer(filename);
}

TEST_F(FFmpegDemuxerTest, save_yuv) {
    std::string yuvname = "F:/Qt_Cpp/road123.mp4";
    // ffmpeg_demuxer_->extract_yuv(yuvname);
}
