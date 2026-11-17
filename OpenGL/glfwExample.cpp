#include <cstdlib>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "GLSL.h"

struct Tri {
    glm::vec3 a, b, c;
};

void appendVertex(std::vector<float>& buffer, const glm::vec3& unitPos, float radius)
{
    glm::vec3 pos = unitPos * radius;

    // Position (location = 0)
    buffer.push_back(pos.x);
    buffer.push_back(pos.y);
    buffer.push_back(pos.z);

    // Normal (location = 1) — unitPos already has length 1.0
    buffer.push_back(unitPos.x);
    buffer.push_back(unitPos.y);
    buffer.push_back(unitPos.z);
}

std::vector<float> buildIcosphereBuffer(float radius, int subdivisions)
{
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f; // what the heck is this golden ratio??

    std::vector<glm::vec3> v = { //icospeohedron or whatever you call it
        glm::normalize(glm::vec3(-1.0f,  t,  0.0f)),
        glm::normalize(glm::vec3( 1.0f,  t,  0.0f)),
        glm::normalize(glm::vec3(-1.0f, -t,  0.0f)),
        glm::normalize(glm::vec3( 1.0f, -t,  0.0f)),

        glm::normalize(glm::vec3( 0.0f, -1.0f,  t)),
        glm::normalize(glm::vec3( 0.0f,  1.0f,  t)),
        glm::normalize(glm::vec3( 0.0f, -1.0f, -t)),
        glm::normalize(glm::vec3( 0.0f,  1.0f, -t)),

        glm::normalize(glm::vec3( t,  0.0f, -1.0f)),
        glm::normalize(glm::vec3( t,  0.0f,  1.0f)),
        glm::normalize(glm::vec3(-t,  0.0f, -1.0f)),
        glm::normalize(glm::vec3(-t,  0.0f,  1.0f))
    };

    std::vector<Tri> faces = {
        {v[0], v[11], v[5]},  {v[0], v[5], v[1]},   {v[0], v[1], v[7]},   {v[0], v[7], v[10]},  {v[0], v[10], v[11]},
        {v[1], v[5], v[9]},   {v[5], v[11], v[4]},  {v[11], v[10], v[2]}, {v[10], v[7], v[6]},  {v[7], v[1], v[8]},
        {v[3], v[9], v[4]},   {v[3], v[4], v[2]},   {v[3], v[2], v[6]},   {v[3], v[6], v[8]},   {v[3], v[8], v[9]},
        {v[4], v[9], v[5]},   {v[2], v[4], v[11]},  {v[6], v[2], v[10]},  {v[8], v[6], v[7]},   {v[9], v[8], v[1]}
    };

    for (int s = 0; s < subdivisions; ++s) {
        std::vector<Tri> nextFaces;
        nextFaces.reserve(faces.size() * 4);

        for (const Tri& tri : faces) {
            glm::vec3 ab = glm::normalize(tri.a + tri.b);
            glm::vec3 bc = glm::normalize(tri.b + tri.c);
            glm::vec3 ca = glm::normalize(tri.c + tri.a);

            nextFaces.push_back({tri.a, ab, ca});
            nextFaces.push_back({tri.b, bc, ab});
            nextFaces.push_back({tri.c, ca, bc});
            nextFaces.push_back({ab, bc, ca});
        }

        faces = std::move(nextFaces);
    }

    std::vector<float> buffer;
    buffer.reserve(faces.size() * 3 * 6);

    for (const Tri& tri : faces) {
        appendVertex(buffer, tri.a, radius);
        appendVertex(buffer, tri.b, radius);
        appendVertex(buffer, tri.c, radius);
    }

    return buffer;
}

int CheckGLErrors(const char *s)
{
    int errCount = 0;
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        const char* errName = "UNKNOWN_ERROR";
        switch (err) {
            case GL_INVALID_ENUM:                  errName = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 errName = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             errName = "GL_INVALID_OPERATION"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: errName = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
            case GL_OUT_OF_MEMORY:                 errName = "GL_OUT_OF_MEMORY"; break;
        }
        std::cerr << "OpenGL Error [" << s << "]: " << errName
                  << " (0x" << std::hex << err << std::dec << ")" << std::endl;
        errCount++;
    }
    return errCount;
}

int main(void)
{
    /* Initialize the library */
    if (!glfwInit()) {
        exit (-1);
    }
    // throw std::runtime_error("Error! initialization of glfw failed!");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);  //required to get the windows going
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    int winWidth = 1000;
    float aspectRatio = 1.0; // 16.0 / 9.0; // winWidth / (float)winHeight;
    int winHeight = winWidth / aspectRatio;
    
    GLFWwindow* window = glfwCreateWindow(winWidth, winHeight, "ThisIsTheWindow", NULL, NULL);
    if (!window) {
        std::cerr << "GLFW did not create a window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if(err != GLEW_OK) {
        std::cerr <<"GLEW Error! glewInit failed, exiting."<< std::endl;
        exit(EXIT_FAILURE);
    }

    const GLubyte* renderer = glGetString (GL_RENDERER);
    const GLubyte* version = glGetString (GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.0, 0.7, 1.0, 1.0);

    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);//interesting, so theres no call back I wonder how the window constantly gets resized

    // Need to set a projection matrix that fits the aspect ratio set
    // by the window frame
    //
    // The ortho parameters, in order: left, right, bottom, top, zNear, zFar
    float halfWidth = 15.0 / 2.0;
    float halfHeight = halfWidth / aspectRatio;
    glm::mat4 projectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, -10.0f, 10.0f);

    GLint major_version;
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    std::cout << "GL_MAJOR_VERSION: " << major_version << std::endl;
    
    // The ortho parameters, in order: left, right, bottom, top, zNear, zFar
    float OrthohalfWidth = 15.0 / 2.0; //screen size
    float OrthohalfHeight = OrthohalfWidth;

    float left = -OrthohalfWidth;
    float right = OrthohalfWidth;

    float bottom = -OrthohalfHeight;
    float top = OrthohalfHeight;

    float near = 5.0f;
    float far = -5.0f;

    glm::mat4 M_ortho = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, near, far); 
    
    glm::mat4 M_perspective = glm::perspective(glm::radians(90.0f), aspectRatio, 1.0f, 100.0f); //I wonder what this matrix looks like, like how do you swap the z into the w with a matrix??


    glm::mat4 model_matrix = glm::mat4(1.0f); //identity matrix I believe?
    

    
    sivelab::GLSLObject siveShader;

    GLuint m_VAO = 0; //int id references vertex array object
    
    glGenVertexArrays(1, &m_VAO); //give the VAO a name
                                  //
    GLuint m_triangleVBO = 0; 
    glGenBuffers(1, &m_triangleVBO); //creates 1
    glBindBuffer(GL_ARRAY_BUFFER, m_triangleVBO);

    // starts reading it 
    
    std::vector<float> host_VertexBuffer = {-3.0f, -3.0f, 0.0f,
                                            3.0f, -3.0f,0.0f, 
                                            0.0f,3.0f, 0.0f,
                                            1.0f,0.0f,0.0f, 
                                            0.0f,1.0f,0.0f,
                                            0.0f,0.0f,1.0f}; //le data
    
                                            
    host_VertexBuffer = buildIcosphereBuffer( 1.0f, 3); //yeah im not doing all that again to make a new triangle
    GLsizei sphereVertexCount = static_cast<GLsizei>(host_VertexBuffer.size() / 6);
    
    glBufferData(GL_ARRAY_BUFFER, host_VertexBuffer.size() * sizeof(float), host_VertexBuffer.data(), GL_STATIC_DRAW); //puts it in gpu
    glBindBuffer(GL_ARRAY_BUFFER, 0); //binds nothing
    
    host_VertexBuffer.clear(); //safe to death

    glBindVertexArray(m_VAO); //start reading this VAO
    
    glEnableVertexAttribArray(0); //flip on attributes
    glEnableVertexAttribArray(1); 

    glBindBuffer(GL_ARRAY_BUFFER, m_triangleVBO); //move the reading head to the vbo
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0); 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(9 * sizeof(GLfloat)));// what makes a GLfloat different concretely? what is a void*
    //aatribute to read
    // components per vertex, in this case xyz and the rgb
    //using floats
    // not normalized
    //size of data to move the range around
    // yup its the offset
    
    glBindVertexArray(0);

    siveShader.addShader("OpenGL/VertexShaderWithMatrix.glsl", sivelab::GLSLObject::ShaderType::VERTEX_SHADER);

    siveShader.addShader("OpenGL/Lambertian.glsl", sivelab::GLSLObject::ShaderType::FRAGMENT_SHADER);

    siveShader.createProgram();

    // screenspace <- cameraspace <- worldspace <- localspace 
    GLint projMatrixID = siveShader.createUniform("projMatrix"); //called after so that it has a ojbect to "link to"
    GLint modelMatrixID = siveShader.createUniform("modelMatrix");
    GLint viewMatrixID = siveShader.createUniform("viewMatrix");
    GLint lightPosWorldID = siveShader.createUniform("lightPosWorld");

        //what is glm???
    glm::vec3 m_pos(0,0,3.0), m_viewDir(0,0,-1);
    glm::vec3 m_U(1,0,0), m_V(0,1,0), m_W(0,0,1);
    
    double timeDiff = 0.0, startFrameTime = 0.0, endFrameTime = 0.0;
//ok bug?

    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        endFrameTime = glfwGetTime();
        timeDiff = endFrameTime - startFrameTime; //is this delta? as in the same as godot?
        startFrameTime = glfwGetTime();

        // Clear the window's buffer (or clear the screen to our
        // background color)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        /* Render your objects here */

        glm::mat4 M_view = glm::lookAt(m_pos, m_pos - m_W, m_V);
        siveShader.activate();
        
        model_matrix = glm::rotate(model_matrix, glm::radians((float)timeDiff * 30.0f), glm::vec3(0.0f,1.0f,0.0f));
        
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(model_matrix));
        glUniformMatrix4fv(projMatrixID, 1, GL_FALSE, glm::value_ptr(M_perspective));
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, glm::value_ptr(M_view));

        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
        glBindVertexArray(0); // binds nothing
        siveShader.deactivate();

        // Swap the front and back buffers
        glfwSwapBuffers(window);

        /* Poll for and process events */ //like buttons and stuff too :)
        glfwPollEvents();
        
        float moveRatePerFrame = 0.05;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
          m_pos = m_pos + -m_W * moveRatePerFrame;
        } else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
          m_pos = m_pos - m_U * moveRatePerFrame;
        } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
          m_pos = m_pos + m_W * moveRatePerFrame;
        } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
          m_pos = m_pos + m_U * moveRatePerFrame;
        }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
       std::cout << "fps: " << 1.0 / timeDiff << std::endl;
    }

        if (glfwGetKey( window, GLFW_KEY_T ) == GLFW_PRESS) {
            std::cout << "fps: " << 1.0/timeDiff << std::endl;
        }
        if (glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, 1);
        }
    }
  
    glfwTerminate();
    return 0;
}