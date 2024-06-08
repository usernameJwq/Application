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
    void show_window();

private:
    void init_glfw();
    bool init_glad();
    void create_window();
    void render();

private:
    static void frame_buffersize_callback(GLFWwindow* window, int width, int height);

private:
    void bind_image_texture(const std::string& img_path);
    void bind_pos_color_source();
    void program_attach_shader();

private:
    GLFWwindow* window_;

    std::string window_title_;
    int window_width_;
    int window_height_;

    GLuint vao_;
    GLuint program_;
};

#endif // !IMAGE_TEXTURE_H_
