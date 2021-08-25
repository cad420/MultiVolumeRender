//
// Created by wyz on 2021/8/23.
//

#include "MultiIsoSurfaceRender.hpp"
#include "Common/Camera.hpp"
#include "Common/ShaderProgram.hpp"
#include "Common/Utils.hpp"
#include "Shaders.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <functional>

class MultiIsoSurfaceRenderImpl
{
  public:
    MultiIsoSurfaceRenderImpl(int w, int h);

    void SetScalarFieldData(ScalarFieldData, ScalarFieldData);

    void SetIsoValueAndColor(double iso_value1, const std::array<double, 4> &color1, double iso_value2,
                             const std::array<double, 4> &color2);

    void Render();

    ~MultiIsoSurfaceRenderImpl();

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
    double iso_value1, iso_value2;
    std::array<double, 4> color1, color2;
    GLuint volume_tex1, volume_tex2;
    float voxel;
    std::array<uint32_t, 3> volume_dim;
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

MultiIsoSurfaceRenderImpl::MultiIsoSurfaceRenderImpl(int w, int h) : window_w(w), window_h(h)
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

void MultiIsoSurfaceRenderImpl::SetScalarFieldData(ScalarFieldData data1, ScalarFieldData data2)
{
    if (data1.x != data2.x || data1.y != data2.y || data1.z != data2.z)
        throw std::runtime_error("Scalar filed data's dim not equal!");

    glGenTextures(1, &volume_tex1);
    glBindTexture(GL_TEXTURE_3D, volume_tex1);
    // need to binding texture unit
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    float color[4] = {0.f, 0.f, 0.f, 0.f};
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, color);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, data1.x, data1.y, data1.z, 0, GL_RED, GL_UNSIGNED_BYTE, data1.data.data());

    glGenTextures(1, &volume_tex2);
    glBindTexture(GL_TEXTURE_3D, volume_tex2);
    // need to binding texture unit
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, color);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, data2.x, data2.y, data2.z, 0, GL_RED, GL_UNSIGNED_BYTE, data2.data.data());

    volume_dim = {data1.x, data1.y, data1.z};
    voxel = 1.f / std::max({volume_dim[0], volume_dim[1], volume_dim[2]});
    auto min_space=std::min({data1.space_x,data1.space_y,data1.space_z});
    space_ratio={data1.space_x/min_space,data1.space_y/min_space,data1.space_z/min_space};
    camera = std::make_unique<TrackBallCamera>(0.5f, window_w, window_h, glm::vec3{0.5f*volume_dim[0]*voxel*space_ratio[0],
                                                                                   0.5f*volume_dim[1]*voxel*space_ratio[1],
                                                                                   0.5f*volume_dim[2]*voxel*space_ratio[2]});
    setProxyCube();
}

void MultiIsoSurfaceRenderImpl::SetIsoValueAndColor(double iso_value1, const std::array<double, 4> &color1,
                                                    double iso_value2, const std::array<double, 4> &color2)
{
    this->iso_value1 = iso_value1;
    this->color1 = color1;
    this->iso_value2 = iso_value2;
    this->color2 = color2;
}

void MultiIsoSurfaceRenderImpl::bindShaderUniform()
{
    glBindTextureUnit(0, volume_tex1);
    glBindTextureUnit(1, volume_tex2);
    glBindTextureUnit(2, raycast_entry_pos_tex);
    glBindTextureUnit(3, raycast_exit_pos_tex);
    raycast_render_shader->use();
    raycast_render_shader->setInt("volume_data1", 0);
    raycast_render_shader->setInt("volume_data2", 1);

    raycast_render_shader->setFloat("voxel", voxel);
    raycast_render_shader->setFloat("step", voxel * 0.3f);
    raycast_render_shader->setVec4("bg_color", 0.f, 0.f, 0.f, 1.f);
    raycast_render_shader->setVec3("volume_board",volume_dim[0]*voxel*space_ratio[0],
                                   volume_dim[1]*voxel*space_ratio[1],
                                   volume_dim[2]*voxel*space_ratio[2]);
    raycast_render_shader->setVec3("space_ratio",space_ratio[0],space_ratio[1],space_ratio[2]);

    raycast_render_shader->setFloat("ka", 0.3f);
    raycast_render_shader->setFloat("kd", 0.7f);
    raycast_render_shader->setFloat("shininess", 100.f);
    raycast_render_shader->setFloat("ks", 0.6f);

    raycast_render_shader->setFloat("iso_value1", iso_value1);
    raycast_render_shader->setFloat("iso_value2", iso_value2);
    raycast_render_shader->setVec4("color1", color1[0], color1[1], color1[2], color1[3]);
    raycast_render_shader->setVec4("color2", color2[0], color2[1], color2[2], color2[3]);
}

void MultiIsoSurfaceRenderImpl::Render()
{
    bindShaderUniform();
    GL_CHECK
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwPollEvents();

        glm::mat4 view = camera->getViewMatrix();
        glm::mat4 projection =
            glm::perspective(glm::radians(camera->getZoom()), (float)window_w / (float)window_h, 0.0001f, 6.0f);
        glm::mat4 mvp = projection * view;

        raycast_pos_shader->use();
        raycast_pos_shader->setMat4("MVPMatrix", mvp);
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

MultiIsoSurfaceRenderImpl::~MultiIsoSurfaceRenderImpl()
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
    glDeleteTextures(1, &volume_tex1);
    glDeleteTextures(1, &volume_tex2);
}

void MultiIsoSurfaceRenderImpl::initGL()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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

void MultiIsoSurfaceRenderImpl::setEventsCallBack()
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

void MultiIsoSurfaceRenderImpl::setProxyCube()
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

void MultiIsoSurfaceRenderImpl::setScreenQuad()
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

void MultiIsoSurfaceRenderImpl::setRaycastPosFramebuffer()
{
    glGenFramebuffers(1, &raycast_pos_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, raycast_pos_fbo);

    glGenTextures(1, &raycast_entry_pos_tex);
    glBindTexture(GL_TEXTURE_2D, raycast_entry_pos_tex);
    glTextureStorage2D(raycast_entry_pos_tex, 1, GL_RGBA32F, window_w, window_h);
    glBindImageTexture(0, raycast_entry_pos_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, raycast_entry_pos_tex, 0);

    glGenRenderbuffers(1, &raycast_pos_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, raycast_pos_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_w, window_h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, raycast_pos_rbo);

    glGenTextures(1, &raycast_exit_pos_tex);
    glBindTexture(GL_TEXTURE_2D, raycast_exit_pos_tex);
    glTextureStorage2D(raycast_exit_pos_tex, 1, GL_RGBA32F, window_w, window_h);
    glBindImageTexture(1, raycast_exit_pos_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, raycast_exit_pos_tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("Framebuffer object is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//------------------------------------------------------------------
MultiIsoSurfaceRender::MultiIsoSurfaceRender(int w, int h)
{
    impl = std::make_unique<MultiIsoSurfaceRenderImpl>(w, h);
}

void MultiIsoSurfaceRender::SetScalarFieldData(ScalarFieldData data1, ScalarFieldData data2)
{
    impl->SetScalarFieldData(std::move(data1), std::move(data2));
}

void MultiIsoSurfaceRender::SetIsoValueAndColor(double iso_value1, const std::array<double, 4> &color1,
                                                double iso_value2, const std::array<double, 4> &color2)
{
    impl->SetIsoValueAndColor(iso_value1, color1, iso_value2, color2);
}

void MultiIsoSurfaceRender::Render()
{
    impl->Render();
}

MultiIsoSurfaceRender::~MultiIsoSurfaceRender()
{
    impl.reset(nullptr);
}
