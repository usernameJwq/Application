#include "image_texture.h"

#include "stb_image.h"

#include "../base/log.h"

ImageTexture::ImageTexture() {}

ImageTexture::~ImageTexture() {}

void ImageTexture::bind_image_texture(const std::string& img_path, GLuint& texture) {
    // 反转 y轴
    stbi_set_flip_vertically_on_load(true);
    // 读取图片
    int width, height, channels;
    stbi_uc* data = stbi_load(img_path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!data) {
        LOG::error("stbi_load image failed");
        return;
    }

    // 生成绑定纹理
    glGenTextures(1, &texture);
    glActiveTexture(texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // 传输纹理数据
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // 释放 img 空间
    stbi_image_free(data);

    // 纹理过滤:
    // 渲染像素 > 采样像素(图片像素) Linear
    // 渲染像素 < 采样像素(图片像素) Nearest
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // 纹理包裹方式
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // u
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // v
}