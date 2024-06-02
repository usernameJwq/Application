#include "glfw_base.h"

#include "../base/log.h"

GlfwBase::GlfwBase()
    : window_(nullptr), window_title_("Glfw Window"), window_width_(600), window_height_(400), vao_(-1), program_(-1) {}

GlfwBase::~GlfwBase() { glfwTerminate(); }

void GlfwBase::init_glfw() { // glfw ��ʼ������
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
    // �������ڣ����õ�ǰ�ı�����
    window_ = glfwCreateWindow(window_width_, window_height_, window_title_.c_str(), NULL, NULL);
    if (!window_) {
        LOG::error("glfwCreateWindow failed");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window_);
}

void GlfwBase::bind_triangle_source() {
    float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};

    GLuint vbo = -1;
    glGenBuffers(1, &vbo);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // �� vbo
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // ��� vbo ��
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
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0);\n"
        "}\0";

    // ���� shader
    GLuint vertex_shader = -1;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertext_shader_source, NULL);
    glCompileShader(vertex_shader);

    // ƬԪ shader
    GLuint fragment_shader = -1;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    program_ = glCreateProgram();

    // program ���� shader
    glAttachShader(program_, vertex_shader);
    glAttachShader(program_, fragment_shader);
    glLinkProgram(program_);

    // ɾ�� shader
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void GlfwBase::render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // ��Ⱦͼ��
    glUseProgram(program_);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
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

    // �ӿڴ�С
    glViewport(0, 0, window_width_, window_height_);

    // ���ûص��¼�
    glfwSetFramebufferSizeCallback(window_, frame_buffersize_callback);
    glfwSetKeyCallback(window_, key_callback);

    bind_triangle_source();
    program_attach_shader();

    while (!(glfwWindowShouldClose(window_))) {
        glfwPollEvents();
        render();

        // ��������
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