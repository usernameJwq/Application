﻿#include "Application.h"

#include "../base/log.h"
#include "../opengl/glfw_base.h"
#include "../opengl/image_texture.h"

#define APP_VERSION "1.0.0"

int main() {
    init_logger();
    LOG::info("APP VERSION {}", APP_VERSION);

    // GlfwBase glfw_base;
    // glfw_base.show_window();

    ImageTexture img_texture;
    img_texture.show_window();

    return 0;
}
