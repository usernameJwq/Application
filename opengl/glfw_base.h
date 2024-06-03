#ifndef GLFW_BASE_H_
#define GLFW_BASE_H_

#include <iostream>
#include <string>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

class GlfwBase {
public:
    GlfwBase();
    ~GlfwBase();

public:
    void show_window();

private:
    void init_glfw();
    bool init_glad();
    void create_window();
    void render();

private:
    // 彩色三角形
    void bind_color_triangle_source();
    void program_attach_color_shader();

    // 纯色三角形
    void bind_triangle_source();
    void program_attach_shader();

private:
    static void frame_buffersize_callback(GLFWwindow* window, int width, int height);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    GLFWwindow* window_;

    std::string window_title_;
    int window_width_;
    int window_height_;

    GLuint vao_;
    GLuint program_;
};

#endif // !GLFW_BASE_H_
