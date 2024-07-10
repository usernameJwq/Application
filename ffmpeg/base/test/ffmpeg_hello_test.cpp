#include <iostream>

#include "gtest/gtest.h"

#include "../ffmpeg_hello.h"

class FFmpegHelloTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(FFmpegHelloTest, ffmpeg_hello) { print_ffmpeg_hello(); }
