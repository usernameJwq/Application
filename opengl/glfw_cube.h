#ifndef GLFW_CUBE_H_
#define GLFW_CUBE_H_

#include <string>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

class GlfwCube {
public:
    GlfwCube();
    ~GlfwCube();

public:
    void show_cube_window();

private:
    void glfw_init();
    bool glad_init();
    void create_cube_window();
    void create_cube_shader();
    void bind_cube_resource();
    void cube_transform();
    void render();

private:
    std::string window_title_;
    int window_width_;
    int window_height_;

    GLFWwindow* cube_window_;

    glm::mat4 trans_mat_;

    GLuint cube_vao_;
    GLuint cube_program_;
};

#endif // ! GLFW_CUBE_H_
