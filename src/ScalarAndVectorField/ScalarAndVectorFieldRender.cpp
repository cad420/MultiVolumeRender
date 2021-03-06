//
// Created by wyz on 2021/8/23.
//

#include "ScalarAndVectorFieldRender.hpp"
#include "Common/Camera.hpp"
#include "Common/ShaderProgram.hpp"
#include "Common/Utils.hpp"
#include "Shaders.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <functional>

class ScalarAndVectorFieldRenderImpl
{
  public:
    ScalarAndVectorFieldRenderImpl(int w, int h);

    void SetScalarFieldData(ScalarFieldData);

    void SetVectorFieldDAta(VectorFieldData);

    void SetTransferFunc(TransferFunc);

    void Render();

    ~ScalarAndVectorFieldRenderImpl();

  private:
    void initGL();

    void setEventsCallBack();

    void setProxyCube();

    void setScreenQuad();

    void setRaycastPosFramebuffer();

    void bindShaderUniform();

  private:
    std::unique_ptr<TrackBallCamera> camera;
    int window_w, window_h;
    std::unique_ptr<Shader> raycast_pos_shader;
    std::unique_ptr<Shader> raycast_render_shader;
    GLFWwindow *window;
    GLuint proxy_cube_vao, proxy_cube_vbo, proxy_cube_ebo;
    GLuint screen_quad_vao, screen_quad_vbo;
    GLuint raycast_pos_fbo, raycast_pos_rbo;
    GLuint raycast_entry_pos_tex, raycast_exit_pos_tex;
    GLuint line_vao, line_vbo, line_ebo;
    GLuint tf_tex;
    GLuint volume_tex;
    float voxel = 0.f;
    std::array<uint32_t, 3> volume_dim;
    GLuint line_num;
    std::array<float, 3> space_ratio;
};

std::function<void(GLFWwindow *window, int width, int height)> framebuffer_resize_callback;

static void glfw_framebuffer_resize_callback(GLFWwindow *window, int width, int height)
{
    framebuffer_resize_callback(window, width, height);
}

std::function<void(GLFWwindow *window, int button, int action, int mods)> mouse_button_callback;

static void glfw_mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    mouse_button_callback(window, button, action, mods);
}

std::function<void(GLFWwindow *window, double xpos, double ypos)> mouse_move_callback;

static void glfw_mouse_move_callback(GLFWwindow *window, double xpos, double ypos)
{
    mouse_move_callback(window, xpos, ypos);
}

std::function<void(GLFWwindow *window, double xoffset, double yoffset)> scroll_callback;

static void glfw_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    scroll_callback(window, xoffset, yoffset);
}

std::function<void(GLFWwindow *window, int key, int scancode, int action, int mods)> keyboard_callback;

static void glfw_keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    keyboard_callback(window, key, scancode, action, mods);
}

ScalarAndVectorFieldRenderImpl::ScalarAndVectorFieldRenderImpl(int w, int h) : window_w(w), window_h(h)
{
    initGL();

    raycast_pos_shader = std::make_unique<Shader>();
    raycast_pos_shader->setShader(shader::raycast_pos_v, shader::raycast_pos_f);
    raycast_render_shader = std::make_unique<Shader>();
    raycast_render_shader->setShader(shader::raycast_render_v, shader::raycast_render_f);

    setEventsCallBack();

    setScreenQuad();
    setRaycastPosFramebuffer();
}

void ScalarAndVectorFieldRenderImpl::SetScalarFieldData(ScalarFieldData data)
{
    glGenTextures(1, &volume_tex);
    glBindTexture(GL_TEXTURE_3D, volume_tex);
    // need to binding texture unit
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    float color[4] = {0.f, 0.f, 0.f, 0.f};
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, color);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, data.x, data.y, data.z, 0, GL_RED, GL_UNSIGNED_BYTE, data.data.data());
    volume_dim = {data.x, data.y, data.z};
    voxel = 1.f / std::max({volume_dim[0], volume_dim[1], volume_dim[2]});
    auto min_space=std::min({data.space_x,data.space_y,data.space_z});
    space_ratio={data.space_x/min_space,data.space_y/min_space,data.space_z/min_space};
    camera = std::make_unique<TrackBallCamera>(0.5f, window_w, window_h, glm::vec3{0.5f*volume_dim[0]*voxel*space_ratio[0],
                                                                                   0.5f*volume_dim[1]*voxel*space_ratio[1],
                                                                                   0.5f*volume_dim[2]*voxel*space_ratio[2]});
    setProxyCube();
}

void ScalarAndVectorFieldRenderImpl::SetVectorFieldDAta(VectorFieldData data)
{
    if (voxel == 0.f)
        throw std::runtime_error("ScalarFieldData should set before!");
    for (auto &v : data.vertices)
    {
        v[0] *= voxel;
        v[1] *= voxel;
        v[2] *= voxel;
    }
    line_num = data.indices.size();
    glGenVertexArrays(1, &line_vao);
    glGenBuffers(1, &line_vbo);
    glGenBuffers(1, &line_ebo);

    glBindVertexArray(line_vao);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
    glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(std::array<float, 3>), data.vertices.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, line_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(uint32_t), data.indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void ScalarAndVectorFieldRenderImpl::SetTransferFunc(TransferFunc tf)
{
    glGenTextures(1, &tf_tex);
    glBindTexture(GL_TEXTURE_1D, tf_tex);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, TF_DIM, 0, GL_RGBA, GL_FLOAT, tf.GetColorTable().data());
}

void ScalarAndVectorFieldRenderImpl::bindShaderUniform()
{
//    glBindTextureUnit(0, tf_tex);
//    glBindTextureUnit(1, volume_tex);
//    glBindTextureUnit(2, raycast_entry_pos_tex);
//    glBindTextureUnit(3, raycast_exit_pos_tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D,tf_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D,volume_tex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_RECTANGLE,raycast_entry_pos_tex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_RECTANGLE,raycast_exit_pos_tex);
    raycast_render_shader->use();
    raycast_render_shader->setInt("transfer_func", 0);
    raycast_render_shader->setInt("volume_data", 1);
    raycast_render_shader->setInt("entry_pos",2);
    raycast_render_shader->setInt("exit_pos",3);

    raycast_render_shader->setFloat("voxel", voxel);
    raycast_render_shader->setFloat("step", voxel * 0.3f);
    raycast_render_shader->setVec3("volume_board",volume_dim[0]*voxel*space_ratio[0],
                                   volume_dim[1]*voxel*space_ratio[1],
                                   volume_dim[2]*voxel*space_ratio[2]);
    raycast_render_shader->setVec3("space_ratio",space_ratio[0],space_ratio[1],space_ratio[2]);

    raycast_render_shader->setVec4("bg_color", 0.f, 0.f, 0.f, 1.f);
    raycast_render_shader->setVec4("line_color", 1.f, 0.f, 0.f, 1.f);

    raycast_render_shader->setFloat("ka", 0.3f);
    raycast_render_shader->setFloat("kd", 0.7f);
    raycast_render_shader->setFloat("shininess", 100.f);
    raycast_render_shader->setFloat("ks", 0.6f);
}

void ScalarAndVectorFieldRenderImpl::Render()
{
    bindShaderUniform();
    GL_CHECK
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwPollEvents();

        glm::mat4 view = camera->getViewMatrix();
        glm::mat4 projection =
            glm::perspective(glm::radians(camera->getZoom()), (float)window_w / (float)window_h, 0.0001f, 6.0f);
        glm::mat4 mvp = projection * view;

        raycast_pos_shader->use();
        raycast_pos_shader->setMat4("MVPMatrix", mvp);
        raycast_pos_shader->setBool("is_line", false);
        glBindFramebuffer(GL_FRAMEBUFFER, raycast_pos_fbo);
        glBindVertexArray(proxy_cube_vao);

        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glDisable(GL_CULL_FACE);

        raycast_pos_shader->setBool("is_line", true);
        glBindVertexArray(line_vao);
        glLineWidth(3);
        glDrawElements(GL_LINES, line_num, GL_UNSIGNED_INT, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        raycast_render_shader->use();

        glBindVertexArray(screen_quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        GL_CHECK

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }
    glfwTerminate();
}

ScalarAndVectorFieldRenderImpl::~ScalarAndVectorFieldRenderImpl()
{
    glDeleteVertexArrays(1, &proxy_cube_vao);
    glDeleteBuffers(1, &proxy_cube_vbo);
    glDeleteBuffers(1, &proxy_cube_ebo);
    glDeleteVertexArrays(1, &screen_quad_vao);
    glDeleteBuffers(1, &screen_quad_vbo);
    glDeleteRenderbuffers(1, &raycast_pos_rbo);
    glDeleteFramebuffers(1, &raycast_pos_fbo);
    glDeleteTextures(1, &raycast_entry_pos_tex);
    glDeleteTextures(1, &raycast_exit_pos_tex);
    glDeleteVertexArrays(1, &line_vao);
    glDeleteBuffers(1, &line_vbo);
    glDeleteBuffers(1, &line_ebo);
    glDeleteTextures(1, &tf_tex);
    glDeleteTextures(1, &volume_tex);
}

void ScalarAndVectorFieldRenderImpl::initGL()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(window_w, window_h, "SingleScalarFieldRender", NULL, NULL);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSwapInterval(1);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glEnable(GL_DEPTH_TEST);

    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_resize_callback);
    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
    glfwSetCursorPosCallback(window, glfw_mouse_move_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);
    glfwSetKeyCallback(window, glfw_keyboard_callback);
}

void ScalarAndVectorFieldRenderImpl::setEventsCallBack()
{

    framebuffer_resize_callback = [&](GLFWwindow *window, int width, int height) -> void {
        glViewport(0, 0, width, height);
    };

    mouse_button_callback = [&](GLFWwindow *window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            double x_pos, y_pos;
            glfwGetCursorPos(window, &x_pos, &y_pos);
            camera->processMouseButton(CameraDefinedMouseButton::Left, true, x_pos, y_pos);
        }
    };

    mouse_move_callback = [&](GLFWwindow *window, double xpos, double ypos) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            camera->processMouseMove(xpos, ypos);
        }
    };

    scroll_callback = [&](GLFWwindow *window, double xoffset, double yoffset) { camera->processMouseScroll(yoffset); };

    keyboard_callback = [&](GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }
    };
}

void ScalarAndVectorFieldRenderImpl::setProxyCube()
{
    std::array<std::array<GLfloat, 3>, 8> proxy_cube_vertices;
    proxy_cube_vertices[0] = {0.f, 0.f, 0.f};
    proxy_cube_vertices[1] = {volume_dim[0] * voxel*space_ratio[0], 0.f, 0.f};
    proxy_cube_vertices[2] = {volume_dim[0] * voxel*space_ratio[0], volume_dim[1] * voxel*space_ratio[1], 0.f};
    proxy_cube_vertices[3] = {0.f, volume_dim[1] * voxel*space_ratio[1], 0.f};
    proxy_cube_vertices[4] = {0.f, 0.f, volume_dim[2] * voxel*space_ratio[2]};
    proxy_cube_vertices[5] = {volume_dim[0] * voxel*space_ratio[0], 0.f, volume_dim[2] * voxel*space_ratio[2]};
    proxy_cube_vertices[6] = {volume_dim[0] * voxel*space_ratio[0], volume_dim[1] * voxel*space_ratio[1], volume_dim[2] * voxel*space_ratio[2]};
    proxy_cube_vertices[7] = {0.f, volume_dim[1] * voxel*space_ratio[1], volume_dim[2] * voxel*space_ratio[2]};

    std::array<GLuint, 36> proxy_cube_vertex_indices = {0, 1, 2, 0, 2, 3, 0, 4, 1, 4, 5, 1, 1, 5, 6, 6, 2, 1,
                                                        6, 7, 2, 7, 3, 2, 7, 4, 3, 3, 4, 0, 4, 7, 6, 4, 6, 5};

    glGenVertexArrays(1, &proxy_cube_vao);
    glGenBuffers(1, &proxy_cube_vbo);
    glGenBuffers(1, &proxy_cube_ebo);
    glBindVertexArray(proxy_cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, proxy_cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(proxy_cube_vertices), proxy_cube_vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, proxy_cube_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(proxy_cube_vertex_indices), proxy_cube_vertex_indices.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void ScalarAndVectorFieldRenderImpl::setScreenQuad()
{
    std::array<GLfloat, 24> screen_quad_vertices = {
        -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};

    glGenVertexArrays(1, &screen_quad_vao);
    glGenBuffers(1, &screen_quad_vbo);
    glBindVertexArray(screen_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, screen_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screen_quad_vertices), screen_quad_vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void ScalarAndVectorFieldRenderImpl::setRaycastPosFramebuffer()
{
    glGenFramebuffers(1, &raycast_pos_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, raycast_pos_fbo);

    glGenTextures(1, &raycast_entry_pos_tex);
    glBindTexture(GL_TEXTURE_RECTANGLE, raycast_entry_pos_tex);
    glTexImage2D(GL_TEXTURE_RECTANGLE,0,GL_RGBA32F,window_w,window_h,0,GL_RGBA,GL_FLOAT,nullptr);
//    glTextureStorage2D(raycast_entry_pos_tex, 1, GL_RGBA32F, window_w, window_h);
//    glBindImageTexture(0, raycast_entry_pos_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, raycast_entry_pos_tex, 0);

    glGenRenderbuffers(1, &raycast_pos_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, raycast_pos_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_w, window_h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, raycast_pos_rbo);

    glGenTextures(1, &raycast_exit_pos_tex);
    glBindTexture(GL_TEXTURE_RECTANGLE, raycast_exit_pos_tex);
//    glTextureStorage2D(raycast_exit_pos_tex, 1, GL_RGBA32F, window_w, window_h);
    glTexImage2D(GL_TEXTURE_RECTANGLE,0,GL_RGBA32F,window_w,window_h,0,GL_RGBA,GL_FLOAT,nullptr);
//    glBindImageTexture(1, raycast_exit_pos_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, raycast_exit_pos_tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("Framebuffer object is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//--------------------------------------------------------------------------------
ScalarAndVectorFieldRender::ScalarAndVectorFieldRender(int w, int h)
{
    impl = std::make_unique<ScalarAndVectorFieldRenderImpl>(w, h);
}

void ScalarAndVectorFieldRender::SetScalarFieldData(ScalarFieldData data)
{
    impl->SetScalarFieldData(std::move(data));
}

void ScalarAndVectorFieldRender::SetVectorFieldDAta(VectorFieldData data)
{
    impl->SetVectorFieldDAta(std::move(data));
}

void ScalarAndVectorFieldRender::SetTransferFunc(TransferFunc tf)
{
    impl->SetTransferFunc(std::move(tf));
}

void ScalarAndVectorFieldRender::Render()
{
    impl->Render();
}

ScalarAndVectorFieldRender::~ScalarAndVectorFieldRender()
{
    impl.reset(nullptr);
}
