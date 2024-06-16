#include "glfw_cube.h"

#include <thread>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "../base/log.h"

GlfwCube::GlfwCube()
    : window_title_("Cube Window")
    , cube_window_(nullptr)
    , trans_mat_(glm::mat4(1.0f))
    , window_width_(600)
    , window_height_(400)
    , cube_vao_(-1)
    , cube_program_(-1) {}

GlfwCube::~GlfwCube() {}

void GlfwCube::glfw_init() {
    glfwInit();
    glfwInitHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwInitHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwInitHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwInitHint(GLFW_RESIZABLE, GL_FALSE);
}

bool GlfwCube::glad_init() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG::error("Cube glad_init failed");
        glfwTerminate();
        return false;
    }
    return true;
}

void GlfwCube::create_cube_window() {
    cube_window_ = glfwCreateWindow(window_width_, window_height_, window_title_.c_str(), NULL, NULL);
    if (!cube_window_) {
        LOG::error("glfwCreateWindow failed");
        return;
    }
    glfwMakeContextCurrent(cube_window_);
}

void GlfwCube::create_cube_shader() {
    const char* vertex_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "out vec3 Color;\n"
        "uniform mat4 transMat;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos, 1.0f) * transMat;\n"
        "   Color = aColor;\n"
        "}\n";

    const char* frag_shader_source =
        "#version 330 core\n"
        "in vec3 Color;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(Color, 1.0f);\n"
        "}\n";

    cube_program_ = glCreateProgram();

    GLuint vertex_shader = -1;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    GLuint frag_shader = -1;
    frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &frag_shader_source, NULL);
    glCompileShader(frag_shader);

    glAttachShader(cube_program_, vertex_shader);
    glAttachShader(cube_program_, frag_shader);
    glLinkProgram(cube_program_);

    glDeleteShader(vertex_shader);
    glDeleteShader(frag_shader);
}

void GlfwCube::bind_cube_resource() {
    const float vertices[] = {
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f,  0.5f,  -0.5f, 1.0f, 0.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f,  -0.5f, 1.0f, 0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f,  0.0f, 1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, 0.0f, -0.5f, -0.5f, 0.5f,  0.0f, 1.0f, 0.0f,

        -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, 1.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, 1.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.0f,
        0.5f,  -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
        0.5f,  -0.5f, 0.5f,  1.0f, 1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
        0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 1.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,  1.0f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,

        -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 0.0f, 1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, 1.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &cube_vao_);
    glBindVertexArray(cube_vao_);

    GLuint vbo = -1;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void GlfwCube::cube_transform() {
    float time = glfwGetTime();

    trans_mat_ = glm::mat4(1.0f);

    // 移动
    // trans_mat_ = glm::translate(trans_mat_, glm::vec3(0.5f * cos(time), 0.5f * sin(time), 0.0f));

    // 旋转
    trans_mat_ = glm::rotate(trans_mat_, time, glm::vec3(0.5f, 0.5f, 0.0f));

    // 缩放
    trans_mat_ = glm::scale(trans_mat_, glm::vec3(0.6f, 0.6f, 0.6f));
}

void GlfwCube::render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(cube_program_);
    glBindVertexArray(cube_vao_);

    GLuint trans_location = glGetUniformLocation(cube_program_, "transMat");
    glUniformMatrix4fv(trans_location, 1, GL_FALSE, glm::value_ptr(trans_mat_));

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);
}

void GlfwCube::show_cube_window() {
    glfw_init();
    create_cube_window();
    bool ret = glad_init();
    if (!ret) {
        LOG::error("glad_init failed");
    }
    glEnable(GL_DEPTH_TEST);

    create_cube_shader();
    bind_cube_resource();

    while (!glfwWindowShouldClose(cube_window_)) {
        glfwPollEvents();
        cube_transform();
        render();
        glfwSwapBuffers(cube_window_);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    glfwTerminate();
}