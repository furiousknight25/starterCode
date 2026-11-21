#include <cstdlib>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "model_obj.h"
#include "GLSL.h"

// Simple triangle struct for sphere subdivision
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

    // Normal (location = 1) - unitPos is already length 1
    buffer.push_back(unitPos.x);
    buffer.push_back(unitPos.y);
    buffer.push_back(unitPos.z);
}

std::vector<float> buildIcosphereBuffer(float radius, int subdivisions)
{
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f; // what the heck is this golden ratio??

    std::vector<glm::vec3> v = { // icospeohedron or whatever you call it
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

int main(int argc, char* argv[])
{
    /* Initialize GLFW */
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    /* Create window */
    int winWidth = 1000;
    float aspectRatio = 1.0f;
    int winHeight = (int)(winWidth / aspectRatio);

    GLFWwindow* window = glfwCreateWindow(winWidth, winHeight, "GLSL Shaders - Lab", NULL, NULL);
    if (!window) {
        std::cerr << "GLFW did not create a window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW Error! glewInit failed, exiting." << std::endl;
        exit(EXIT_FAILURE);
    }

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // Background color
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);

    // Triangle is in the XY plane facing towards +Z, so normal is (0, 0, 1)
    std::vector<float> triangleBuffer = {
        //  x,     y,    z,      nx,   ny,   nz
        -2.0f, -2.0f, 0.0f,     0.0f, 0.0f, 1.0f,
         2.0f, -2.0f, 0.0f,     0.0f, 0.0f, 1.0f,
         0.0f,  2.0f, 0.0f,     0.0f, 0.0f, 1.0f
    };

    GLuint m_triangleVAO = 0;
    GLuint m_triangleVBO = 0;
    glGenVertexArrays(1, &m_triangleVAO);
    glGenBuffers(1, &m_triangleVBO);

    glBindVertexArray(m_triangleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, triangleBuffer.size() * sizeof(float), triangleBuffer.data(), GL_STATIC_DRAW);

    // Attribute 0 = Position (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);

    // Attribute 1 = Normal (vec3)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glBindVertexArray(0);

    std::vector<float> sphereBuffer = buildIcosphereBuffer(1.0f, 3);
    GLsizei sphereVertexCount = static_cast<GLsizei>(sphereBuffer.size() / 6);

    GLuint m_sphereVAO = 0;
    GLuint m_sphereVBO = 0;
    glGenVertexArrays(1, &m_sphereVAO);
    glGenBuffers(1, &m_sphereVBO);

    glBindVertexArray(m_sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereBuffer.size() * sizeof(float), sphereBuffer.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glBindVertexArray(0);

    std::string objFilename = "examples/scenes/scenes_B/objFiles/bunny.obj";
    if (argc > 1) {
        objFilename = argv[1];
    }

    ModelOBJ objModel;
    bool hasObj = objModel.import(objFilename.c_str(), true);
    GLuint m_objVAO = 0;
    GLuint m_objVBO = 0;
    GLsizei objVertexCount = 0;

    if (hasObj) {
        objModel.normalize(2.5f, true);

        const ModelOBJ::Vertex* verts = objModel.getVertexBuffer();
        const int* indices = objModel.getIndexBuffer();
        int numIndices = objModel.getNumberOfIndices();

        std::vector<float> objBuffer;
        objBuffer.reserve(numIndices * 6);

        for (int i = 0; i < numIndices; ++i) {
            const ModelOBJ::Vertex& v = verts[indices[i]];
            // Position
            objBuffer.push_back(v.position[0]);
            objBuffer.push_back(v.position[1]);
            objBuffer.push_back(v.position[2]);
            // Normal
            objBuffer.push_back(v.normal[0]);
            objBuffer.push_back(v.normal[1]);
            objBuffer.push_back(v.normal[2]);
        }

        objVertexCount = static_cast<GLsizei>(objBuffer.size() / 6);

        glGenVertexArrays(1, &m_objVAO);
        glGenBuffers(1, &m_objVBO);

        glBindVertexArray(m_objVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_objVBO);
        glBufferData(GL_ARRAY_BUFFER, objBuffer.size() * sizeof(float), objBuffer.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

        glBindVertexArray(0);

        std::cout << "Loaded OBJ model: " << objFilename << " (" << objModel.getNumberOfTriangles() << " triangles)" << std::endl;
    } else {
        std::cout << "Note: Could not open " << objFilename << ", OBJ mesh option will be skipped." << std::endl;
    }

    sivelab::GLSLObject lambertShader;
    lambertShader.addShader("OpenGL/VertexShaderWithMatrix.glsl", sivelab::GLSLObject::ShaderType::VERTEX_SHADER);
    lambertShader.addShader("OpenGL/Lambertian.glsl", sivelab::GLSLObject::ShaderType::FRAGMENT_SHADER);
    lambertShader.createProgram();

    sivelab::GLSLObject blinnShader;
    blinnShader.addShader("OpenGL/VertexShaderWithMatrix.glsl", sivelab::GLSLObject::ShaderType::VERTEX_SHADER);
    blinnShader.addShader("OpenGL/BlinnPhong.glsl", sivelab::GLSLObject::ShaderType::FRAGMENT_SHADER);
    blinnShader.createProgram();

    sivelab::GLSLObject normalShader;
    normalShader.addShader("OpenGL/VertexShaderWithMatrix.glsl", sivelab::GLSLObject::ShaderType::VERTEX_SHADER);
    normalShader.addShader("OpenGL/NormalShader.glsl", sivelab::GLSLObject::ShaderType::FRAGMENT_SHADER);
    normalShader.createProgram();

    sivelab::GLSLObject* siveShader = &lambertShader;

    glm::vec3 m_pos(0.0f, 0.0f, 6.0f);
    glm::vec3 m_U(1.0f, 0.0f, 0.0f);
    glm::vec3 m_V(0.0f, 1.0f, 0.0f);
    glm::vec3 m_W(0.0f, 0.0f, 1.0f);

    int currentMesh = 0;        // 0: One Triangle, 1: Two Triangles , 2: Sphere, 3: OBJ
    int totalMeshes = hasObj ? 4 : 3;
    bool usePerspective = true; // Toggle P for ortho / perspective

    double startFrameTime = glfwGetTime();
    double timeDiff = 0.0;
    float rotationAngle = 0.0f;

    std::cout << "  1: Switch to Lambertian Shader" << std::endl;
    std::cout << "  2: Switch to Blinn-Phong Shader" << std::endl;
    std::cout << "  3: Switch to Normal Shader" << std::endl;
    std::cout << "  M: Toggle Mesh (Single Triangle -> Two Triangles -> Sphere" << (hasObj ? " -> OBJ Model)" : ")") << std::endl;
    std::cout << "  P: Toggle Perspective / Orthographic projection" << std::endl;
    std::cout << "  W/A/S/D: Move camera" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;

    // Key debounce state
    bool mKeyPrev = false;
    bool pKeyPrev = false;

    while (!glfwWindowShouldClose(window))
    {
        double endFrameTime = glfwGetTime();
        timeDiff = endFrameTime - startFrameTime;
        startFrameTime = endFrameTime;

        rotationAngle += (float)timeDiff * 25.0f;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        siveShader->activate();

        // Projection Matrix
        glm::mat4 M_proj;
        if (usePerspective) {
            M_proj = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 100.0f);
        } else {
            float halfW = 4.0f;
            float halfH = halfW / aspectRatio;
            M_proj = glm::ortho(-halfW, halfW, -halfH, halfH, 0.1f, 100.0f);
        }

        // View Matrix
        glm::mat4 M_view = glm::lookAt(m_pos, m_pos - m_W, m_V);

        // Light position in world space
        glm::vec4 lightPosWorld = glm::vec4(3.0f, 4.0f, 5.0f, 1.0f);
        glm::vec4 eyePosWorld = glm::vec4(m_pos, 1.0f);

        GLint projMatrixID = siveShader->createUniform("projMatrix");
        GLint modelMatrixID = siveShader->createUniform("modelMatrix");
        GLint normalMatrixID = siveShader->createUniform("normalMatrix");
        GLint viewMatrixID = siveShader->createUniform("viewMatrix");
        GLint lightPosWorldID = siveShader->createUniform("lightPosWorld");
        GLint eyePosWorldID = siveShader->createUniform("eyePosWorld");
        GLint diffuseComponentID = siveShader->createUniform("diffuseComponent");
        GLint diffuseColorID = siveShader->createUniform("diffuseColor");
        GLint specularColorID = siveShader->createUniform("specularColor");
        GLint specularExponentID = siveShader->createUniform("specularExponent");
        GLint sinPosID = siveShader->createUniform("sinPos");

        // Pass common uniforms
        glUniformMatrix4fv(projMatrixID, 1, GL_FALSE, glm::value_ptr(M_proj));
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, glm::value_ptr(M_view));
        glUniform4fv(lightPosWorldID, 1, glm::value_ptr(lightPosWorld));
        glUniform4fv(eyePosWorldID, 1, glm::value_ptr(eyePosWorld));

        glUniform3f(specularColorID, 1.0f, 1.0f, 1.0f);
        glUniform1f(specularExponentID, 32.0f);
        glUniform1f(sinPosID, rotationAngle);
        
        // Render selected geometry
        if (currentMesh == 0) {
            // Mesh 0: Single Triangle (Step 1-4)
            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 normalMat = glm::transpose(glm::inverse(model));

            glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, glm::value_ptr(normalMat));
            glUniform3f(diffuseComponentID, 0.9f, 0.5f, 0.2f);
            glUniform3f(diffuseColorID, 0.9f, 0.5f, 0.2f);

            glBindVertexArray(m_triangleVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);

        } else if (currentMesh == 1) {
            // Mesh 1: Two Triangles (Step 3 Question 6)
            // Triangle 1: Blue at (-2, 1, 0)
            glm::mat4 model1 = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.5f, 0.0f));
            model1 = glm::scale(model1, glm::vec3(0.7f));
            glm::mat4 normMat1 = glm::transpose(glm::inverse(model1));

            glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(model1));
            glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, glm::value_ptr(normMat1));
            glUniform3f(diffuseComponentID, 0.1f, 0.4f, 0.9f); // Blue
            glUniform3f(diffuseColorID, 0.1f, 0.4f, 0.9f);

            glBindVertexArray(m_triangleVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);

            // Triangle 2: Red at (2, -1, 0)
            glm::mat4 model2 = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -0.5f, 0.0f));
            model2 = glm::scale(model2, glm::vec3(0.7f));
            glm::mat4 normMat2 = glm::transpose(glm::inverse(model2));

            glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(model2));
            glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, glm::value_ptr(normMat2));
            glUniform3f(diffuseComponentID, 0.9f, 0.2f, 0.2f); // Red
            glUniform3f(diffuseColorID, 0.9f, 0.2f, 0.2f);

            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);

        } else if (currentMesh == 2) {
            // Mesh 2: Sphere with rotation
            glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 normalMat = glm::transpose(glm::inverse(model));

            glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, glm::value_ptr(normalMat));
            glUniform3f(diffuseComponentID, 0.9f, 0.5f, 0.2f);
            glUniform3f(diffuseColorID, 0.9f, 0.5f, 0.2f);

            glBindVertexArray(m_sphereVAO);
            glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
            glBindVertexArray(0);

        } else if (currentMesh == 3 && hasObj) {
            glm::mat4 model_base = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f,0.0f,1.0f));
            glm::mat4 model = glm::rotate(model_base, glm::radians(rotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 normalMat = glm::transpose(glm::inverse(model));

            glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(normalMatrixID, 1, GL_FALSE, glm::value_ptr(normalMat));
            glUniform3f(diffuseComponentID, 0.8f, 0.7f, 0.6f);
            glUniform3f(diffuseColorID, 0.8f, 0.7f, 0.6f);

            glBindVertexArray(m_objVAO);
            glDrawArrays(GL_TRIANGLES, 0, objVertexCount);
            glBindVertexArray(0);
        }

        siveShader->deactivate();

        glfwSwapBuffers(window);
        glfwPollEvents();

        // Camera movement controls
        float moveRate = 3.0f * (float)timeDiff;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            m_pos = m_pos - m_W * moveRate;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            m_pos = m_pos + m_W * moveRate;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            m_pos = m_pos - m_U * moveRate;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            m_pos = m_pos + m_U * moveRate;
        }

        // Shader switches: 1, 2, 3
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            siveShader = &lambertShader;
            std::cout << "Lambertian" << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            siveShader = &blinnShader;
            std::cout << "Blinn-Phong" << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
            siveShader = &normalShader;
            std::cout << "Normal" << std::endl;
        }

        // Toggle Mesh with 'M'
        bool mKeyPressed = (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS);
        if (mKeyPressed && !mKeyPrev) {
            currentMesh = (currentMesh + 1) % totalMeshes;
            if (currentMesh == 0) std::cout << " Single Triangle" << std::endl;
            if (currentMesh == 1) std::cout << " Two Triangles " << std::endl;
            if (currentMesh == 2) std::cout << " Rotating Sphere" << std::endl;
            if (currentMesh == 3) std::cout << " OBJ Model" << std::endl;
        }
        mKeyPrev = mKeyPressed;

        // Toggle Perspective / Ortho with 'P'
        bool pKeyPressed = (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS);
        if (pKeyPressed && !pKeyPrev) {
            usePerspective = !usePerspective;
            std::cout << "[Projection] " << (usePerspective ? "Perspective" : "Orthographic") << std::endl;
        }
        pKeyPrev = pKeyPressed;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, 1);
        }
    }

    // Cleanup
    glDeleteBuffers(1, &m_triangleVBO);
    glDeleteVertexArrays(1, &m_triangleVAO);
    glDeleteBuffers(1, &m_sphereVBO);
    glDeleteVertexArrays(1, &m_sphereVAO);
    if (hasObj) {
        glDeleteBuffers(1, &m_objVBO);
        glDeleteVertexArrays(1, &m_objVAO);
    }

    glfwTerminate();
    return 0;
}
