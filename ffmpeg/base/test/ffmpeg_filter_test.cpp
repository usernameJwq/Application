#include <iostream>

#include "gtest/gtest.h"

#include "../ffmpeg_filter.h"

class FFmpegFilterTest : public ::testing::Test {
protected:
    void SetUp() override { ffmpeg_filter_ = std::make_unique<FFmpegFilter>(); };

    std::unique_ptr<FFmpegFilter> ffmpeg_filter_;
};

TEST_F(FFmpegFilterTest, ffmpeg_filter) {
    std::string filename = "F:/Qt_Cpp/road123.mp4";
    std::string desc = "fps=25";
    // TODO: 添加滤镜描述时错误
    // ffmpeg_filter_->open_filter_file(filename, desc);
}
