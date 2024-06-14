#ifndef FFMPEG_BASE_H_
#define FFMPEG_BASE_H_

extern "C" {
#include "libavcodec/avcodec.h"
}

class FFmpegBase {
public:
    FFmpegBase();
    ~FFmpegBase();

private:
};

#endif // ! FFMPEG_BASE_H_
