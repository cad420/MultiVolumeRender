//
// Created by wyz on 2021/9/23.
//
#include "Common/Camera.hpp"
#include "Common/ShaderProgram.hpp"
#include "Common/Utils.hpp"
#include "OceanMultiScalarFieldRender.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <functional>
class OceanMultiScalarFieldRenderImpl{
  public:
    OceanMultiScalarFieldRenderImpl(int w,int h);

    ~OceanMultiScalarFieldRenderImpl();

    void SetOceanScalarFieldData(OceanScalarData,OceanScalarData);

    void SetTransferFunc(TransferFunc,TransferFunc);

    void Render();

  private:
    void initGL();

    void setEventsCallBack();

    void setProxyCube();

    void setScreenQuad();

    void setRaycastPosFramebuffer();

    void bindShaderUniform();

  private:
    std::unique_ptr<TrackBallCamera> camera;
    int window_w,window_h;
    std::unique_ptr<Shader> raycast_pos_shader;
    std::unique_ptr<Shader> raycast_render_shader;
    GLFWwindow* window;
    GLuint proxy_cube_vao, proxy_cube_vbo, proxy_cube_ebo;
    GLuint screen_quad_vao, screen_quad_vbo;
    GLuint raycast_pos_fbo, raycast_pos_rbo;
    GLuint raycast_entry_pos_tex, raycast_exit_pos_tex;
    GLuint tf_tex1,tf_tex2;
    GLuint volume_tex1,volume_tex2;

    std::array<GLint,3> volume_dim;
    std::vector<float> proxy_mesh_vertex;
    std::vector<uint32_t> proxy_mesh_index;
    float min_lon,min_lat,min_dist;
    float len_lon,len_lat,len_dist;
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


OceanMultiScalarFieldRenderImpl::OceanMultiScalarFieldRenderImpl(int w, int h)
:window_w(w),window_h(h)
{
    initGL();

    raycast_pos_shader=std::make_unique<Shader>("C:\\Users\\wyz\\projects\\MultiVolumeRender\\src\\OceanMultiScalarField\\Shader\\raycast_pos_v.glsl",
                                                  "C:\\Users\\wyz\\projects\\MultiVolumeRender\\src\\OceanMultiScalarField\\Shader\\raycast_pos_f.glsl");
    raycast_render_shader=std::make_unique<Shader>("C:\\Users\\wyz\\projects\\MultiVolumeRender\\src\\OceanMultiScalarField\\Shader\\raycast_render_v.glsl",
                                                     "C:\\Users\\wyz\\projects\\MultiVolumeRender\\src\\OceanMultiScalarField\\Shader\\raycast_render_f.glsl");

    camera=std::make_unique<TrackBallCamera>(1.f*OceanScalarData::earth_radius,window_w,window_h,glm::vec3(0.f,0.f,0.f));
    setScreenQuad();
    setRaycastPosFramebuffer();
    setEventsCallBack();
}
void OceanMultiScalarFieldRenderImpl::SetOceanScalarFieldData(OceanScalarData data1,OceanScalarData data2)
{
    {
        auto dim1=data1.GetDataShape();
        auto dim2=data2.GetDataShape();
        if(!std::equal(dim1.begin(),dim1.end(),dim2.begin(),dim2.end())){
            throw std::runtime_error("OceanScalarData's dim is not equal.");
        }
    }
    {
        data1.ReOrder();
        volume_dim = data1.GetDataShape();
        data1.GetDataAreaRange(min_lon, min_lat, min_dist, len_lon, len_lat, len_dist);
        data1.GenerateBoundaryMesh();
        this->proxy_mesh_vertex = std::move(data1.GetBoundaryVertices());
        this->proxy_mesh_index = std::move(data1.GetBoundaryIndices());

        glGenTextures(1, &volume_tex1);
        glBindTexture(GL_TEXTURE_3D, volume_tex1);
        // need to binding texture unit
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        float color[4] = {0.f, 0.f, 0.f, 0.f};
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, color);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, volume_dim[0], volume_dim[1], volume_dim[2], 0, GL_RED, GL_UNSIGNED_BYTE,
                     data1.GetDataArray().data());

        data2.ReOrder();
        glGenTextures(1, &volume_tex2);
        glBindTexture(GL_TEXTURE_3D, volume_tex2);
        // need to binding texture unit
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, color);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, volume_dim[0], volume_dim[1], volume_dim[2], 0, GL_RED, GL_UNSIGNED_BYTE,
                     data2.GetDataArray().data());
    }
    setProxyCube();
}
void OceanMultiScalarFieldRenderImpl::SetTransferFunc(TransferFunc tf1,TransferFunc tf2)
{
    glGenTextures(1, &tf_tex1);
    glBindTexture(GL_TEXTURE_1D, tf_tex1);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, TF_DIM, 0, GL_RGBA, GL_FLOAT, tf1.GetColorTable().data());

    glGenTextures(1, &tf_tex2);
    glBindTexture(GL_TEXTURE_1D, tf_tex2);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, TF_DIM, 0, GL_RGBA, GL_FLOAT, tf2.GetColorTable().data());
}
void OceanMultiScalarFieldRenderImpl::bindShaderUniform()
{

    glBindTextureUnit(0, tf_tex1);
    glBindTextureUnit(1, tf_tex2);
    glBindTextureUnit(2, volume_tex1);
    glBindTextureUnit(3, volume_tex2);
    GL_CHECK
    glBindTextureUnit(4, raycast_entry_pos_tex);
    glBindTextureUnit(5, raycast_exit_pos_tex);
    GL_CHECK
    raycast_render_shader->use();
    raycast_render_shader->setInt("transfer_func1", 0);
    raycast_render_shader->setInt("transfer_func2", 1);
    raycast_render_shader->setInt("volume_data1", 2);
    raycast_render_shader->setInt("volume_data2", 3);
    raycast_render_shader->setInt("entry_pos",4);
    raycast_render_shader->setInt("exit_pos",5);


    raycast_render_shader->setFloat("min_lon",min_lon);
    raycast_render_shader->setFloat("min_lat",min_lat);
    raycast_render_shader->setFloat("min_dist",min_dist);
    raycast_render_shader->setFloat("len_lon",len_lon);
    raycast_render_shader->setFloat("len_lat",len_lat);
    raycast_render_shader->setFloat("len_dist",len_dist);
    raycast_render_shader->setFloat("step",300.f);

    LOG_INFO("min dist {0}",min_dist);
}
void OceanMultiScalarFieldRenderImpl::Render()
{
    bindShaderUniform();
    GL_CHECK
    while(!glfwWindowShouldClose(window)){
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwPollEvents();
        glm::mat4 view = camera->getViewMatrix();
        glm::mat4 projection =
            glm::perspective(glm::radians(camera->getZoom()), (float)window_w / (float)window_h,
                             0.0001f*OceanScalarData::earth_radius, 6.0f*OceanScalarData::earth_radius);
        glm::mat4 mvp = projection * view;

        raycast_pos_shader->use();
        raycast_pos_shader->setMat4("MVPMatrix", mvp);
        glBindFramebuffer(GL_FRAMEBUFFER, raycast_pos_fbo);
        glBindVertexArray(proxy_cube_vao);

        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, proxy_mesh_index.size(), GL_UNSIGNED_INT, 0);

        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CW);
        glDrawBuffer(GL_COLOR_ATTACHMENT1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, proxy_mesh_index.size(), GL_UNSIGNED_INT, 0);
        glDisable(GL_CULL_FACE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        raycast_render_shader->use();
        raycast_render_shader->setVec3("camera_pos",camera->getCameraPos());
        glBindVertexArray(screen_quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        GL_CHECK

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }
    glfwTerminate();
}
OceanMultiScalarFieldRenderImpl::~OceanMultiScalarFieldRenderImpl()
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
    glDeleteTextures(1, &tf_tex1);
    glDeleteTextures(1, &tf_tex2);
    glDeleteTextures(1, &volume_tex1);
    glDeleteTextures(1, &volume_tex2);

}

void OceanMultiScalarFieldRenderImpl::initGL()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(window_w, window_h, "OceanMultiScalarFieldRender", NULL, NULL);
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

void OceanMultiScalarFieldRenderImpl::setProxyCube()
{
    glGenVertexArrays(1, &proxy_cube_vao);
    glGenBuffers(1, &proxy_cube_vbo);
    glGenBuffers(1, &proxy_cube_ebo);
    glBindVertexArray(proxy_cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, proxy_cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, proxy_mesh_vertex.size()*sizeof(float), proxy_mesh_vertex.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, proxy_cube_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, proxy_mesh_index.size()*sizeof(uint32_t), proxy_mesh_index.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void OceanMultiScalarFieldRenderImpl::setScreenQuad()
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

void OceanMultiScalarFieldRenderImpl::setRaycastPosFramebuffer()
{
    glGenFramebuffers(1, &raycast_pos_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, raycast_pos_fbo);

    glGenTextures(1, &raycast_entry_pos_tex);
    glBindTexture(GL_TEXTURE_RECTANGLE, raycast_entry_pos_tex);
    glTexImage2D(GL_TEXTURE_RECTANGLE,0,GL_RGBA32F,window_w,window_h,0,GL_RGBA,GL_FLOAT,nullptr);
    glTexParameteri(GL_TEXTURE_RECTANGLE,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    GL_CHECK
//    glTextureStorage2D(raycast_entry_pos_tex, 1, GL_RGBA32F, window_w, window_h);
//    glBindImageTexture(0, raycast_entry_pos_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, raycast_entry_pos_tex, 0);
    GL_CHECK

    glGenRenderbuffers(1, &raycast_pos_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, raycast_pos_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_w, window_h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, raycast_pos_rbo);

    glGenTextures(1, &raycast_exit_pos_tex);
    glBindTexture(GL_TEXTURE_RECTANGLE, raycast_exit_pos_tex);
    glTexImage2D(GL_TEXTURE_RECTANGLE,0,GL_RGBA32F,window_w,window_h,0,GL_RGBA,GL_FLOAT,nullptr);
    glTexParameteri(GL_TEXTURE_RECTANGLE,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
//    glTextureStorage2D(raycast_exit_pos_tex, 1, GL_RGBA32F, window_w, window_h);
//    glBindImageTexture(1, raycast_exit_pos_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, raycast_exit_pos_tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("Framebuffer object is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_CHECK
}
void OceanMultiScalarFieldRenderImpl::setEventsCallBack()
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

//---------------------------------------------------------------------------------
OceanMultiScalarFieldRender::OceanMultiScalarFieldRender(int w, int h)
{
    impl=std::make_unique<OceanMultiScalarFieldRenderImpl>(w,h);
}
void OceanMultiScalarFieldRender::SetOceanScalarFieldData(OceanScalarData data1,OceanScalarData data2)
{
    impl->SetOceanScalarFieldData(std::move(data1),std::move(data2));
}
void OceanMultiScalarFieldRender::SetTransferFunc(TransferFunc tf1,TransferFunc tf2)
{
    impl->SetTransferFunc(std::move(tf1),std::move(tf2));
}
void OceanMultiScalarFieldRender::Render()
{
    impl->Render();
}
OceanMultiScalarFieldRender::~OceanMultiScalarFieldRender()
{
    impl.reset();
}
