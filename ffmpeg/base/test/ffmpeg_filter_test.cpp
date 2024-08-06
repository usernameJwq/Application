#include <iostream>

#include "gtest/gtest.h"

#include "../ffmpeg_filter.h"

class FFmpegFilterTest : public ::testing::Test {
protected:
    void SetUp() override { ffmpeg_filter_ = std::make_unique<FFmpegFilter>(); };

    std::unique_ptr<FFmpegFilter> ffmpeg_filter_;
};

TEST_F(FFmpegFilterTest, ffmpeg_filter) {
    std::string input_filename = "F:/Qt_Cpp/road123.mp4";
    std::string output_filename = "F:/Qt_Cpp/out.mp4";
    std::string desc = "drawbox=30:30:25:25:red";
    ffmpeg_filter_->add_video_filter(input_filename, output_filename, desc);

    while (true) {
    }
}
