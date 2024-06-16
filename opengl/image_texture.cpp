#include "image_texture.h"

#include "stb_image.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "../base/log.h"

ImageTexture::ImageTexture()
    : window_(nullptr)
    , window_title_("Glfw Window")
    , window_width_(600)
    , window_height_(400)
    , trans_mat_(glm::mat4(1.0f))
    , vao_(-1)
    , program_(-1) {}

ImageTexture::~ImageTexture() {}

void ImageTexture::init_glfw() {
    // glfw 初始化
    glfwInit();
    glfwInitHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwInitHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwInitHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

bool ImageTexture::init_glad() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG::error("gladLoadGLLoader failed");
        glfwTerminate();
        return false;
    }
    return true;
}

void ImageTexture::create_window() {
    // 创建窗口，设置当前文本对象
    window_ = glfwCreateWindow(window_width_, window_height_, window_title_.c_str(), NULL, NULL);
    if (!window_) {
        LOG::error("glfwCreateWindow failed");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window_);
}

void ImageTexture::bind_image_texture(const std::string& img_path) {
    // 反转 y轴
    stbi_set_flip_vertically_on_load(true);
    // 读取图片
    int width, height, channels;
    stbi_uc* data = stbi_load(img_path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!data) {
        LOG::error("stbi_load image failed");
        return;
    }

    // 生成、绑定纹理
    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 自动设置层级关系
    glGenerateMipmap(GL_TEXTURE_2D);

    // 传输纹理数据
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // 手动设置 Minmap 层级渐变
    // int s_width = width;
    // int s_height = height;
    // int level = 0;
    // while (true) {
    //    if (s_width <= 1 || s_height <= 1) {
    //        break;
    //    }

    //    glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, s_width, s_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //    s_width = s_width < 1 ? 1 : s_width / 2;
    //    s_height = s_height < 1 ? 1 : s_height / 2;
    //    level++;
    //}

    // 释放 img 空间
    stbi_image_free(data);

    // 纹理过滤:
    // 渲染像素 > 采样像素(图片像素) Linear 线性差值
    // 渲染像素 < 采样像素(图片像素) Nearest 临近采样
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Minmap 过滤
    // GL_NEAREST_MIPMAP_LINEAR 说明：
    //  GL_NEAREST 单张图片之间的采样方式
    //  MIPMAP_LINEAR 层级之间的采样方式
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);

    // 纹理包裹方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // u
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // v
}

/*
 *  uvs 说明：
 *  u 和 v 范围都是 0~1
 *  1x1 表示原比例 1:1
 *  2x2 表示原比例 2:1 也就是 4 宫格
 */
void ImageTexture::bind_pos_color_source() {
    float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f};
    float colors[] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float uvs[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    int index[] = {0, 1, 2, 2, 1, 3};

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // 绑定 vertext
    GLuint vbo_vertex = -1;
    glGenBuffers(1, &vbo_vertex);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // 绑定 color
    GLuint vbo_fragment = -1;
    glGenBuffers(1, &vbo_fragment);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_fragment);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), &colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));

    // 绑定 uvs
    GLuint uv_vbo = -1;
    glGenBuffers(1, &uv_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), &uvs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // 绑定 ebo
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), &index, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void ImageTexture::program_attach_shader() {
    const char* vertext_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "layout (location = 2) in vec2 aUV;\n"
        "out vec3 color;\n"
        "out vec2 uv;\n"
        "uniform float time;\n"
        "uniform mat4 transFormMat;\n"
        "void main()\n"
        "{\n"

        // 正常坐标播放
        "   gl_Position = vec4(aPos, 1.0f) * transFormMat;\n"
        "   color = aColor;\n"

        // Minmap 比例播放, 改变坐标大小
        //"   float scale = 1.0 / time;\n"
        //"   vec3 sPos = aPos * scale;\n"
        //"   gl_Position = vec4(sPos, 1.0f);\n"

        // 正常显示 UV
        "   uv = aUV;\n"

        // 轮播效果
        //"   float delta = time * 0.3;\n"
        //"   uv = vec2(aUV.x + delta, aUV.y);\n"

        "}\0";

    const char* fragment_shader_source =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 color;\n"
        "in vec2 uv;\n"
        "uniform sampler2D sampler;\n"
        "void main()\n"
        "{\n"
        "   FragColor = texture(sampler, uv);\n"
        "}\0";

    GLuint vertex_shader = -1;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertext_shader_source, NULL);
    glCompileShader(vertex_shader);

    GLuint fragment_shader = -1;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    program_ = glCreateProgram();
    glAttachShader(program_, vertex_shader);
    glAttachShader(program_, fragment_shader);
    glLinkProgram(program_);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

// 对显示的纹理进行旋转、缩放等变换
void ImageTexture::transform_position() {
    // 平移
    trans_mat_ = glm::translate(trans_mat_, glm::vec3(1.0f, 1.0f, 1.0f));

    // 旋转
    trans_mat_ = glm::rotate(trans_mat_, 90.0f, glm::vec3(0.0f, 0.0f, 1.0f));

    // 缩放
    trans_mat_ = glm::scale(trans_mat_, glm::vec3(0.5f, 0.5f, 0.5f));
}

void ImageTexture::render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint sampler_location = glGetUniformLocation(program_, "sampler");
    glUniform1i(sampler_location, 0);

    GLuint time_location = glGetUniformLocation(program_, "time");
    glUniform1f(time_location, glfwGetTime());

    GLuint trans_location = glGetUniformLocation(program_, "transFormMat");
    glUniformMatrix4fv(trans_location, 1, GL_FALSE, glm::value_ptr(trans_mat_));

    glUseProgram(program_);
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0));
    glBindVertexArray(0);
}

void ImageTexture::show_window() {
    init_glfw();
    create_window();

    bool ret = false;
    ret = init_glad();
    if (!ret) {
        LOG::error("init_glad failed");
        return;
    }

    // 视口大小
    glViewport(0, 0, window_width_, window_height_);

    // 函数回调
    glfwSetFramebufferSizeCallback(window_, frame_buffersize_callback);

    program_attach_shader();
    bind_pos_color_source();
    bind_image_texture("F:/Qt_Cpp/cmake_pro/Application/assets/imgs/goku.png");
    transform_position();

    while (!(glfwWindowShouldClose(window_))) {
        // 事件监听
        glfwPollEvents();
        // 图形渲染
        render();
        // 交换缓冲 buffer
        glfwSwapBuffers(window_);
    }

    glfwTerminate();
}

void ImageTexture::frame_buffersize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    glfwSwapBuffers(window);
}