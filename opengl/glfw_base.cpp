#include "glfw_base.h"

#include "../base/log.h"

GlfwBase::GlfwBase()
    : window_(nullptr), window_title_("Glfw Window"), window_width_(600), window_height_(400), vao_(-1), program_(-1) {}

GlfwBase::~GlfwBase() { glfwTerminate(); }

void GlfwBase::init_glfw() {
    // glfw 初始化配置
    glfwInit();
    glfwInitHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwInitHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwInitHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

bool GlfwBase::init_glad() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG::error("gladLoadGLLoader failed");
        glfwTerminate();
        return false;
    }
    return true;
}

void GlfwBase::create_window() {
    // 创建窗口，设置当前文本对象
    window_ = glfwCreateWindow(window_width_, window_height_, window_title_.c_str(), NULL, NULL);
    if (!window_) {
        LOG::error("glfwCreateWindow failed");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window_);
}

void GlfwBase::bind_triangle_source() {
    float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f};
    int indexs[] = {0, 1, 2, 2, 1, 3};

    GLuint vbo = -1;
    glGenBuffers(1, &vbo);

    GLuint ebo = -1;
    glGenBuffers(1, &ebo);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // 绑定 vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // 绑定 ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexs), &indexs, GL_STATIC_DRAW);
    // 解绑 vao
    glBindVertexArray(0);
}

void GlfwBase::program_attach_shader() {
    const char* vertext_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";

    const char* fragment_shader_source =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\0";

    // 顶点 shader
    GLuint vertex_shader = -1;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertext_shader_source, NULL);
    glCompileShader(vertex_shader);

    // 片元 shader
    GLuint fragment_shader = -1;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    program_ = glCreateProgram();

    // program 附加 shader
    glAttachShader(program_, vertex_shader);
    glAttachShader(program_, fragment_shader);
    glLinkProgram(program_);

    // 删除 shader
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void GlfwBase::bind_color_triangle_source() {
    float vertices[] = {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, -0.5f, 0.0f,
                        0.0f,  1.0f,  0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f,  1.0f};
    int indexs[] = {0, 1, 2};

    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLuint ebo;
    glGenBuffers(1, &ebo);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // 绑定 vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    // 顶点信息
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    // 颜色信息
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    // 绑定 ebo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexs), &indexs, GL_STATIC_DRAW);
    // 释放 vao
    glBindVertexArray(0);
}

void GlfwBase::program_attach_color_shader() {
    const char* vertext_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "out vec3 color;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "   color = aColor;\n"
        "}\0";

    const char* fragment_shader_source =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 color;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(color, 1.0f);\n"
        "}\0";

    // 创建 顶点 shader
    GLuint vertex_shader = -1;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertext_shader_source, NULL);
    glCompileShader(vertex_shader);

    // 创建 片元 shader
    GLuint fragment_shader = -1;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    // 创建 shader 编译器
    program_ = glCreateProgram();
    glAttachShader(program_, vertex_shader);
    glAttachShader(program_, fragment_shader);
    glLinkProgram(program_);

    // 删除 shader
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void GlfwBase::render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program_);
    glBindVertexArray(vao_);

    // 渲染多边形
    // glDrawArrays(GL_TRIANGLES, 0, 3);

    /*
     * glDrawElements 参数 indices 说明：
     *  使用 ebo 默认为 0
     *  使用 ebo 最后一个参数是数字，表示 ebo 偏移量
     *  使用 ebo 可以将 indexs 整个数组传进去
     */
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(0));
}

void GlfwBase::show_window() {
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

    // 设置回调事件
    glfwSetFramebufferSizeCallback(window_, frame_buffersize_callback);
    glfwSetKeyCallback(window_, key_callback);

    // 纯色三角形
    // bind_triangle_source();
    // program_attach_shader();

    // 彩色三角形
    bind_color_triangle_source();
    program_attach_color_shader();

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

void GlfwBase::frame_buffersize_callback(GLFWwindow* window, int width, int height) {
    LOG::trace("frame_buffersize_callback width={}, height={}", width, height);
    glViewport(0, 0, width, height);
    glfwSwapBuffers(window);
}

void GlfwBase::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    LOG::trace("key_callback key={}, scancode={}, mods={}");
}