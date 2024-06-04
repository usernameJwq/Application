#ifndef IMAGE_TEXTURE_H_
#define IMAGE_TEXTURE_H_

#define STB_IMAGE_IMPLEMENTATION

#include <string>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

class ImageTexture {
public:
    ImageTexture();
    ~ImageTexture();

public:
    void bind_image_texture(const std::string& img_path, GLuint& texture);
};

#endif // !IMAGE_TEXTURE_H_
