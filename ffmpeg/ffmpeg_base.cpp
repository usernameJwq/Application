#include "ffmpeg_base.h"

#include <iostream>

#include "../base/log.h"

FFmpegBase::FFmpegBase() { LOG::info("FFmpeg Version {}", av_version_info()); }

FFmpegBase::~FFmpegBase() {}