#ifndef FFMPEG_HELLO_
#define FFMPEG_HELLO_

#include <iostream>

extern "C" {
#include "libavutil/avutil.h"
}

void print_ffmpeg_hello();

#endif // !FFMPEG_HELLO_