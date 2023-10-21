/* Christopher Anderson
 * Prof. Montalvo-Ruiz
 * Comp Graphic and Visualization
 * 5-5 Milestone: Texturing Objects in a 3D Scene
 * 9-30-2023
 */

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gl/gl.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility functions

#include <iostream>
#include <cstdlib>      
#include "camera.h"

using namespace std;

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

//namespace for global variable declarations
namespace
{
    // settings
    const char* const WINDOW_TITLE = "ChristopherAnderson 5-5"; // Macro for window title

    const unsigned int WINDOW_WIDTH = 1500;
    const unsigned int WINDOW_HEIGHT = 1500;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbos[2];     // Handles for the vertex buffer objects
        GLuint nIndices;    // Number of indices of the mesh
    };

    // Triangle mesh object declarations
    GLMesh gMesh;   //helmet mesh object
    GLMesh gMesh2;  //carpet mesh object
    GLMesh gMesh3;  //red head lamp mesh object
    GLMesh gMesh4;  //grey head lamp mesh object
    GLMesh gMesh5; //water bottle body
    GLMesh gMesh6;  //water bottle cap
    GLMesh gMesh7; //arrow shaft
    GLMesh gMesh8; // arrow fletching
    GLMesh gMesh9; //Cam head
    GLMesh gMesh10; // Cam shaft

    GLMesh gMeshLight;

    //Texture id
    GLuint gTextureId; // generic dirty helment texture
    GLuint gTextureId2; // carpet texture
    GLuint gTextureId3; // red headlamp texture
    GLuint gTextureId4; // grey headlamp texture
    GLuint gTextureId5; //water bottle body texture
    GLuint gTextureId6; // water bottle cap texture
    GLuint gTextureId7; // arrow shaft texture
    GLuint gTextureId8; // arrow fletching texture
    GLuint gTextureId9; // Cam head texture
    GLuint gTextureId10; // Cam shaft texture

    glm::vec2 gUVScale(5.0f, 5.0f);

    // Shader program
    GLuint gProgramId;
    GLuint lightingId;
    // primary GLFW window
    GLFWwindow* window = nullptr;

    //lighting variables
    glm::vec3 objectColor(1.0f, 1.0f, 1.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 lightPosition(4.0f, 6.0f, 0.0f);
    glm::vec3 lightScale(0.5f);

}

//All function prototypes
void createHelmetMesh(GLMesh& mesh);
void createCarpetMesh(GLMesh& mesh);
void createRedHLMesh(GLMesh& mesh);
void createGreyHLMesh(GLMesh& mesh);
void createWaterBottleMesh(GLMesh& mesh);
void createBottleCapMesh(GLMesh& mesh);
void createArrowShaftMesh(GLMesh& mesh);
void createArrowFletchingMesh(GLMesh& mesh);
void createCamHeadMesh(GLMesh& mesh);
void createCamShaftMesh(GLMesh& mesh);

void renderPrimary();
bool createTexture(const char* filename, GLuint& textureId);
bool createShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void destroyShaderProgram(GLuint programId);
void destroyTexture(GLuint textureId);
void destroyMesh(GLMesh& mesh);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);


// camera variable declarations/initializations
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;

// timing variables
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//Cylinder object declaration
//Cylinder cylinderWaterBottle(1, 1, 4, 36, true, 2);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
    layout(location = 1) in vec2 textureCoordinate;  // texture data from Vertex Attrib Pointer 1
    layout(location = 2) in vec3 normal; // VAP position 1 for normals
    
    out vec2 vertexTextureCoordinate; // variable to transfer texture data to the fragment shader
    out vec3 vertexNormal; // For outgoing normals to fragment shader
    out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader


    //Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
        
        vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
        vertexTextureCoordinate = textureCoordinate;
           
        vertexTextureCoordinate = textureCoordinate; // references incoming color data
    }
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec3 vertexNormal; // For incoming normals
    in vec3 vertexFragmentPos; // For incoming fragment position
    in vec2 vertexTextureCoordinate;
    
    out vec4 fragmentColor; // For outgoing cube color to the GPU
    
    // Uniform / Global variables for object color, light color, light position, and camera/view position
    uniform vec3 objectColor;
    uniform vec3 lightColor;
    uniform vec3 lightPos;
    uniform vec3 viewPosition;
    uniform sampler2D uTexture; // Useful when working with multiple textures
    uniform vec2 uvScale;
    
    void main()
    {
        //Phong lighting model calculations to generate ambient, diffuse, and specular components
        //Calculate Ambient lighting
        float ambientStrength = 0.7f; // Set ambient or global lighting strength
        vec3 ambient = ambientStrength * lightColor; // Generate ambient light color
    
        //Calculate Diffuse lighting
        vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
        vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
        float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
        vec3 diffuse = impact * lightColor; // Generate diffuse light color
    
        //Calculate Specular lighting
        float specularIntensity = 30.0f; // Set specular light strength
        float highlightSize = 16.0f; // Set specular highlight size
        vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
        vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector

        //Calculate specular component
        float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
        vec3 specular = specularIntensity * specularComponent * lightColor;
    
        // Texture holds the color to be used for all three components
        vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);
    
        // Calculate phong result
        vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;
    
        fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
    }
);

/* Lamp Shader Source Code*/
const GLchar* lightingVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

    //Uniform / Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    }
    );
    
    
/* Fragment Shader Source Code*/
const GLchar* lightingFragmentShaderSource = GLSL(440,
    
    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU
    
    void main()
    {
        fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
    }
);


// Creates a perspective projection
glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
//bool perspectDefault = true;

// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    //define application window parameters
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Christopher Anderson 5-5 Milestone: Texturing Objects in 3D Scene", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // glfw window creation
    // --------------------
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // Create the meshes for all 3D objects
    createHelmetMesh(gMesh); 
    createCarpetMesh(gMesh2);
    createRedHLMesh(gMesh3);
    createGreyHLMesh(gMesh4);
    createWaterBottleMesh(gMesh5);
    createBottleCapMesh(gMesh6);
    createArrowShaftMesh(gMesh7);
    createArrowFletchingMesh(gMesh8);
    createCamHeadMesh(gMesh9);
    createCamShaftMesh(gMesh10);

    // Create the shader program
    if (!createShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Load generic dirty helmet texture as texture0
    const char* texFilename = "../textures/textureSM.jpg";
    if (!createTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Load carpet texture as texture1
    texFilename = "../textures/textureSM2.jpg";
    if (!createTexture(texFilename, gTextureId2))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 1
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture2"), 1);

    //Load red headlamp texture as texture2
    texFilename = "../textures/textureSM3.jpg";
    if (!createTexture(texFilename, gTextureId3))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 2
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture3"), 2);

    // Load grey headlamp texture as texture3
    texFilename = "../textures/textureSM4.jpg";
    if (!createTexture(texFilename, gTextureId4))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 3
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture4"), 3);

    // Load water bottle body texture as texture4
    texFilename = "../textures/textureSM5.jpg";
    if (!createTexture(texFilename, gTextureId5))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 4
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture5"), 4);

    // Load water bottle body texture as texture5
    texFilename = "../textures/textureSM6.jpg";
    if (!createTexture(texFilename, gTextureId6))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 5
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture6"), 5);

    if (!createShaderProgram(lightingVertexShaderSource, lightingFragmentShaderSource, lightingId))
        return EXIT_FAILURE;

    // Load arrow shaft texture as texture6
    texFilename = "../textures/textureSM7.jpg";
    if (!createTexture(texFilename, gTextureId7))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 6
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture7"), 6);

    if (!createShaderProgram(lightingVertexShaderSource, lightingFragmentShaderSource, lightingId))
        return EXIT_FAILURE;

    // Load arrow fletching texture as texture7
    texFilename = "../textures/textureSM8.jpg";
    if (!createTexture(texFilename, gTextureId8))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 7
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture8"), 7);

    // Load Cam head texture as texture8
    texFilename = "../textures/textureSM9.jpg";
    if (!createTexture(texFilename, gTextureId9))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 8
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture9"), 8);

    // Load Cam shaft texture as texture8
    texFilename = "../textures/textureSM10.jpg";
    if (!createTexture(texFilename, gTextureId10))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 8
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture10"), 9);


    if (!createShaderProgram(lightingVertexShaderSource, lightingFragmentShaderSource, lightingId))
        return EXIT_FAILURE;


    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        renderPrimary();
        //renderSecondary();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }



    // Release mesh data
    destroyMesh(gMesh);
    destroyMesh(gMesh2);
    destroyMesh(gMesh3);
    destroyMesh(gMesh4);
    destroyMesh(gMesh5);
    destroyMesh(gMesh6);
    destroyMesh(gMesh7);
    destroyMesh(gMesh8);
    destroyMesh(gMesh9);
    destroyMesh(gMesh10);

    // Release texture
    destroyTexture(gTextureId);
    destroyTexture(gTextureId2);
    destroyTexture(gTextureId3);
    destroyTexture(gTextureId4);
    destroyTexture(gTextureId5);
    destroyTexture(gTextureId6);
    destroyTexture(gTextureId7);
    destroyTexture(gTextureId8);
    destroyTexture(gTextureId9);
    destroyTexture(gTextureId10);

    // Release shader program
    destroyShaderProgram(gProgramId);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// Functioned called to render a frame
void renderPrimary()
{
    const float angularVelocity = glm::radians(45.0f);

    glm::vec4 newPosition = glm::rotate(angularVelocity * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightPosition, 1.0f);
    lightPosition.x = newPosition.x;
    lightPosition.y = newPosition.y;
    lightPosition.z = newPosition.z;
    
    
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Scales the object by 2
    glm::mat4 scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
    // 2. Rotates shape by 15 degrees in the x axis
    glm::mat4 rotation = glm::rotate(0.0F, glm::vec3(4.0f, 4.0f, 4.0f));
    // 3. Place object at the origin
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = translation * rotation * scale;

    // camera/view transformation
    glm::mat4 view = camera.GetViewMatrix();

    // Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");

    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Activate the VBOs contained within the mesh's VAO and draws the triangles
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);
    glBindVertexArray(gMesh.vao);       // call BindVertexArray with gMesh.vao will construct helmet mesh
    glDrawElements(GL_TRIANGLES, gMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

    glBindTexture(GL_TEXTURE_2D, 0);

    //Activate VBOs contained with mesh2 and draw triangles to form mesh2 object
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId2);
    glBindVertexArray(gMesh2.vao);
    glDrawElements(GL_TRIANGLES, gMesh2.nIndices, GL_UNSIGNED_SHORT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);

    //activate mesh #3 and draw mesh using triangle primative
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId3);
    glBindVertexArray(gMesh3.vao);
    glDrawElements(GL_TRIANGLES, gMesh3.nIndices, GL_UNSIGNED_SHORT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);

    //activate mesh #4 and draw mesh using triangle primative
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId4);
    glBindVertexArray(gMesh4.vao);
    glDrawElements(GL_TRIANGLES, gMesh4.nIndices, GL_UNSIGNED_SHORT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);

    
    //activate mesh #5 and draw mesh using triangle primative
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId5);
    glBindVertexArray(gMesh5.vao);
    glDrawElements(GL_TRIANGLES, gMesh5.nIndices, GL_UNSIGNED_SHORT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);

    //activate mesh #6 and draw mesh using triangle primative
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId6);
    glBindVertexArray(gMesh6.vao);
    glDrawElements(GL_TRIANGLES, gMesh6.nIndices, GL_UNSIGNED_SHORT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);
    
    //activate mesh #7 and draw mesh using triangle primative
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId7);
    glBindVertexArray(gMesh7.vao);
    glDrawElements(GL_TRIANGLES, gMesh7.nIndices, GL_UNSIGNED_SHORT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);

    //activate mesh #8 and draw mesh using triangle primative
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId8);
    glBindVertexArray(gMesh8.vao);
    glDrawElements(GL_TRIANGLES, gMesh8.nIndices, GL_UNSIGNED_SHORT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);

    //activate mesh #9 and draw mesh using triangle primative
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId9);
    glBindVertexArray(gMesh9.vao);
    glDrawElements(GL_TRIANGLES, gMesh9.nIndices, GL_UNSIGNED_SHORT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);

    //activate mesh #10 and draw mesh using triangle primative
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId10);
    glBindVertexArray(gMesh10.vao);
    glDrawElements(GL_TRIANGLES, gMesh10.nIndices, GL_UNSIGNED_SHORT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(window);    // Flips the the back buffer with the front buffer every frame.

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, objectColor.r, objectColor.g, objectColor.b);
    glUniform3f(lightColorLoc, lightColor.r, lightColor.g, lightColor.b);
    glUniform3f(lightPositionLoc, lightPosition.x, lightPosition.y, lightPosition.z);
    const glm::vec3 cameraPosition = camera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Draws the triangles
    //glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    glDrawElements(GL_TRIANGLES, gMesh4.nIndices, GL_UNSIGNED_SHORT, NULL);

    // LAMP: draw lamp
    //----------------
    glUseProgram(lightingId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(lightPosition) * glm::scale(lightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(lightingId, "model");
    viewLoc = glGetUniformLocation(lightingId, "view");
    projLoc = glGetUniformLocation(lightingId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms 
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);
    glDrawElements(GL_TRIANGLES, gMesh3.nIndices, GL_UNSIGNED_SHORT, NULL);


    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(window);    // Flips the the back buffer with the front buffer every frame.

}

/*function for light object generation
void createLightingObject(GLMesh& mesh) {

}*/

/*
 * MESH FUNCTIONS
 * ====================
 */

//Climbing Cam Meshes
//function to generate Cam mechanical head mesh
void createCamHeadMesh(GLMesh& mesh) {
    // Position and Color data 
    const float textureRepeats = 0.25;
    GLfloat verts[] = {

        //Cam Head coordinates
        // Vertex Positions                             // Texture Coordinates
        1.0f, -1.0f, 4.0f,               textureRepeats, 0.0f, //v0         
        1.0f, 0.0f, 4.0f,                0.0f, 0.0f, //v1
        2.0f, -0.5f, 4.0f,               0.0f, textureRepeats,//v2

        1.0f, -1.0f, 3.5f,               textureRepeats, textureRepeats,//v3         
        1.0f, 0.0f, 3.5f,               textureRepeats, textureRepeats,//v4
        2.0f, -0.5f, 3.5f,               textureRepeats, textureRepeats//v5

    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 2,         //front of Cam
        3, 4, 5,         //back of Cam
        2, 1, 5,        //top side
        4, 5, 1,
        3, 0, 2,        //bottom side
        5, 2, 3,
        1, 4, 0,        //side-side
        3, 0, 4
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + 1)));
    glEnableVertexAttribArray(1);
}

//function for Cam handle mesh
void createCamShaftMesh(GLMesh& mesh) {
    const float baseX = 1.0f;
    const float endX = -1.0f;
    // Position and Color data 
    const float textureRepeats = 0.1;
    GLfloat verts[] = {

        //rough cylinder prototype
        // Vertex Positions                   y= sqrt(abs(0.01 - pow(double(!!!z!!! - 3.75), 2))) - 0.5                             // Texture Coordinates
        baseX, -0.5f,  3.75f,              textureRepeats, textureRepeats,//v0         // bottom circle to cylinder
        baseX, -0.5f,  3.65f,             textureRepeats, textureRepeats,//v1
        baseX, -1.0 * sqrt(abs(0.01 - pow(double(3.7 - 3.75), 2))) - 0.5, 3.7f ,             textureRepeats, textureRepeats,//v2
        baseX, -1.0 * sqrt(abs(0.01 - pow(double(3.75 - 3.75), 2))) - 0.5,  3.75f,                textureRepeats, textureRepeats,//v3
        baseX, -1.0 * sqrt(abs(0.01 - pow(double(3.8- 3.75), 2))) - 0.5,  3.8,             textureRepeats, textureRepeats,//v4
        baseX, sqrt(abs(0.01 - pow(double(3.85 - 3.75), 2))) - 0.5,   3.85f,                textureRepeats, textureRepeats,//v5
        baseX, sqrt(abs(0.01 - pow(double(3.8 - 3.75), 2))) - 0.5, 3.8f,            textureRepeats, textureRepeats,//v6
        baseX, sqrt(abs(0.01 - pow(double(3.75 - 3.75), 2))) - 0.5, 3.75f,            textureRepeats, textureRepeats,//v7                                                                            
        baseX, sqrt(abs(0.01 - pow(double(3.7 - 3.75), 2))) - 0.5, 3.7f,           textureRepeats, textureRepeats,//v8
        baseX, sqrt(abs(0.01 - pow(double(3.65 - 3.75), 2))) - 0.5,  3.65f,             textureRepeats, textureRepeats,//v9
              
        endX,  -0.5f - .4,    3.75f,                                                               textureRepeats, textureRepeats,//v10     // top circle to cylinder
        endX,  -0.5f - .4,    3.65f,                                                               textureRepeats, textureRepeats,//v11
        endX,  -.5866 - .4/*-1.0f * sqrt(abs(0.01 - pow(double(3.7 - 3.75), 2))) - 0.5f*/,   3.7f,            textureRepeats, textureRepeats,//v12
        endX,  -.6 - .4/*-1.0f * sqrt(abs(0.01 - pow(double(3.75 - 3.75), 2))) - 0.5f */ ,   3.75f,                 textureRepeats, textureRepeats,//v13
        endX,  -.5866 - .4/*-1.0f * sqrt(abs(0.01 - pow(double(3.8 - 3.75), 2))) - 0.5f*/,  3.8f,             textureRepeats, textureRepeats,//v14      
        endX,   -.5 - .4/*sqrt(abs(0.01 - pow(double(3.85 - 3.75), 2))) - 0.5f */ ,   3.85f,                textureRepeats, textureRepeats,//v15
        endX,   -.413397 - .4/*sqrt(abs(0.01 - pow(double(3.8 - 3.75), 2))) - 0.5f*/,  3.8f,             textureRepeats, textureRepeats,//v16
        endX,   -.4 - .4/*sqrt(abs(0.01 - pow(double(3.75 - 3.75), 2))) - 0.5f */ , 3.75f,                textureRepeats, textureRepeats,//v17
        endX,   -.413397 - .4/*sqrt(abs(0.01 - pow(double(3.7 - 3.75), 2))) - 0.5f*/,   3.7f,       textureRepeats, textureRepeats,//v18
        endX,   -.5 - .4/*sqrt(abs(0.01 - pow(double(3.65 - 3.75), 2))) - 0.5f */ ,  3.65f,        textureRepeats, textureRepeats//v19
             
    };

    // Index data to share position data
    //const int elementNumberIndices = 60;
    GLushort indices[] = {
        //bottom circle
        0,1,2,
        0,3,2,
        0,3,4,
        0,4,5,
        0,5,6,
        0,6,7,
        0,7,8,
        0,8,9,

        //top circle
        10,11,12,
        10,13,12,
        10,13,14,
        10,14,15,
        10,15,16,
        10,16,17,
        10,17,18,
        10,18,19,

        // cylinder wall covering half-bottom
        11,1,12,
        2,12,1,

        12,2,13,
        3,13,2,

        13,3,14,
        4,14,3,

        14,4,15,
        5,15,4,

        15,5,16,
        6,16,5,

        16,6,17,
        7,17,6,

        17,7,18,
        8,18,7,

        18,8,19,
        9,19,8
        
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + 1)));
    glEnableVertexAttribArray(1);
}

//Arrow Meshes
//function for Arrow shaft Mesh
void createArrowShaftMesh(GLMesh& mesh) {

    // Position and Color data 
    const float textureRepeats = 1.0;
    GLfloat verts[] = {

        //rough cylinder prototype
        // Vertex Positions                             // Texture Coordinates
        4.0f, -1.0f, 2.0f,               0.0f, 0.0f,//v0         // bottom left of bottom for arrow shaft
        4.25f, -1.0f, 1.75f,             textureRepeats, 0.0f,//v1         // bottom right of bottom for arrow shaft
        4.0f, -0.75f, 2.0f,              0.0f, textureRepeats,//v2         //top left of bottom for arrow shaft
        4.25f, -0.75f, 1.75f,            textureRepeats, textureRepeats,//v3         //top right of bottom for arrow shaft
        
        -1.0f, 4.0f, -2.25f,             0.0f, 0.0f,//v4     // bottom left of top for arrow shaft
        -1.25f, 4.0f, -2.0f,             textureRepeats, 0.0f,//v5      // bottom right of top for arrow shaft
        -1.0f, 4.25f, -2.25f,            0.0f, textureRepeats,//v6     //top left of top for arrow shaft
        -1.25f, 4.25f, -2.0f,            textureRepeats, textureRepeats//v7      //top right of top for arrow shaft
    };

    // Index data to share position data
    GLushort indices[] = {
        //bottom of shaft
        0, 1, 2,
        3, 1, 2,
        //top of shaft
        4, 5, 7,
        6, 7, 4,

        //half cylinder wall covering half-bottom
        1, 0, 5,    //bottom
        1, 3, 4,    //right
        2, 3, 6,    //top
        0, 2, 5,    //left

        //half cylinder wall covering half-top
        4, 5, 1,    //bottom
        5, 7, 2,    //right
        7, 6, 2,    //top
        4, 6, 3     //left
        
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex)));
    glEnableVertexAttribArray(1);
}

//function for Arrow Fetching Mesh
void createArrowFletchingMesh(GLMesh& mesh) {
    // Position and Color data 
    const float textureRepeats = 300.0;
    GLfloat verts[] = {

        //rough cylinder prototype
        // Vertex Positions                             // Texture Coordinates
        4.125f, -0.75f, 1.875f,               textureRepeats, textureRepeats,//v0         // forward attachment
        3.0f, 0.25f, 1.0f,                  textureRepeats, textureRepeats,//v1            //rear attachment
        4.0, 0.0f, 1.875f,                  textureRepeats, textureRepeats,//v2           // 'peak' of fletching

        4.0f, -0.875f, 2.0f,               textureRepeats, textureRepeats,//v3         // forward attachment
        3.0f, 0.15f, 1.125f,                  textureRepeats, textureRepeats,//v4            //rear attachment
        3.0, -0.5f, 2.0f,                  textureRepeats, textureRepeats,//v5           // 'peak' of fletching

        4.25f, -0.875f, 1.75f,          textureRepeats, textureRepeats,//v6         // forward attachment  
        3.0f, 0.15f, 1.0f,             textureRepeats, textureRepeats,//v7            //rear attachment
        4.0f, -0.5f, 0.8f,            textureRepeats, textureRepeats,//v8           // 'peak' of fletching
    };

    // Index data to share position data
    GLushort indices[] = {
        //Top Fletching triangular plane
        0, 1, 2,
        3, 4, 5,
        6, 7, 8

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + 1)));
    glEnableVertexAttribArray(1);
}

//Water Bottle Meshes
 //function to generate water bottle cap mesh object
void createBottleCapMesh(GLMesh& mesh)
{

    const float baseHeight = 3.0f;
    const float topHeight = 3.5f;
    const float diameter = 1.0f;
    // Position and Color data 
    const float textureRepeats = 0.25;
    GLfloat verts[] = {

        //rough cylinder prototype
        // Vertex Positions                //z = sqrt(diameter/2 -(x-4)^2)                                 // Texture Coordinates
        4.0f, baseHeight, 0.0f,                                                                         textureRepeats, textureRepeats,//v0         // bottom circle to cylinder
        3.5f, baseHeight, 0.0f,                                                                      textureRepeats, textureRepeats,//v1
        3.643f, baseHeight,  0.5f * sqrtf(abs(diameter / 2.0 - pow(double(3.643 - 4), 2))),             textureRepeats, textureRepeats,//v2
        3.786f, baseHeight, 0.5f * sqrtf(abs(diameter / 2.0 - pow(double(3.786 - 4), 2))),                textureRepeats, textureRepeats,//v3
        3.929f, baseHeight,  0.5f * sqrtf(abs(diameter / 2.0 - pow(double(3.929 - 4), 2))),             textureRepeats, textureRepeats,//v4
        4.072f, baseHeight, 0.5f * sqrtf(abs(diameter / 2.0 - pow(double(4.072 - 4), 2))),                textureRepeats, textureRepeats,//v5
        4.215f, baseHeight,  0.5f * sqrtf(abs(diameter / 2.0 - pow(double(4.215 - 4), 2))),                   textureRepeats, textureRepeats,//v6
        4.358f, baseHeight, 0.5f * sqrtf(abs(diameter / 2.0 - pow(double(4.358 - 4), 2))),                       textureRepeats, textureRepeats,//v7

                    
        4.5f, baseHeight, 0.0f,                                                                         textureRepeats, textureRepeats,//v8
        4.357f, baseHeight, -0.5f * sqrtf(abs(diameter / 2.0 - pow(double(4.357 - 4), 2))),             textureRepeats, textureRepeats,//v9
        4.214f, baseHeight, -0.5f * sqrtf(abs(diameter / 2.0 - pow(double(4.214 - 4), 2))),              textureRepeats, textureRepeats,//v10
        4.071f, baseHeight, -0.5f * sqrtf(abs(diameter / 2.0 - pow(double(4.071 - 4), 2))),             textureRepeats, textureRepeats,//v11
        3.928f, baseHeight, -0.5f * sqrtf(abs(diameter / 2.0 - pow(double(3.928 - 4), 2))),              textureRepeats, textureRepeats,//v12
        3.785f, baseHeight, -0.5f * sqrtf(abs(diameter / 2.0 - pow(double(3.785 - 4), 2))),             textureRepeats, textureRepeats,//v13
        3.642f, baseHeight, -0.5f * sqrtf(abs(diameter / 2.0 - pow(double(3.642 - 4), 2))),              textureRepeats, textureRepeats,//v14
        3.5f,baseHeight, 0.0f,                                                                           textureRepeats, textureRepeats,//v15
                                               
        4.0f, topHeight, 0.0f,                                                                           textureRepeats, textureRepeats,//v16     // top circle to cylinder
        3.5f, topHeight, 0.0f,                                                                              textureRepeats, textureRepeats,//v17
        3.643f, topHeight, 0.5f * sqrtf(abs(diameter / 2.0 - pow(double(3.643 - 4), 2))),             textureRepeats, textureRepeats,//v18
        3.786f, topHeight, 0.5f * sqrtf(abs(diameter / 2.0 - pow(double(3.786 - 4), 2))),                textureRepeats, textureRepeats,//v19
        3.929f, topHeight,  0.5f * sqrtf(abs(diameter / 2.0 - pow(double(3.929 - 4), 2))),            0.0f, textureRepeats,//v20
        4.072f, topHeight, 0.5f * sqrtf(abs(diameter / 2.0 - pow(double(4.072 - 4), 2))),                textureRepeats, textureRepeats,//v21
        4.215f, topHeight,  0.5f * sqrtf(abs(diameter / 2.0 - pow(double(4.215 - 4), 2))),             textureRepeats, textureRepeats,//v22
        4.358f, topHeight, 0.5f * sqrtf(abs(diameter / 2.0 - pow(double(4.358 - 4), 2))),                textureRepeats, textureRepeats,//v23

                        
        4.5f, topHeight, 0.0f,                                                                      textureRepeats, textureRepeats,//v24
        4.357f, topHeight, -0.5f * sqrtf(abs(diameter / 2.0 - pow(double(4.357 - 4), 2))),           textureRepeats, textureRepeats,//v25
        4.214f, topHeight, -0.5f * sqrtf(abs(diameter / 2.0 - pow(double(4.214 - 4), 2))),          textureRepeats, textureRepeats,//v26
        4.071f, topHeight, -0.5f * sqrtf(abs(diameter / 2.0 - pow(double(4.0 - 4), 2))),           textureRepeats, textureRepeats,//v27
        3.928f, topHeight, -0.5f * sqrtf(abs(diameter / 2.0 - pow(double(3.928 - 4), 2))),          textureRepeats, textureRepeats,//v28
        3.785f, topHeight, -0.5f * sqrtf(abs(diameter / 2.0 - pow(double(3.785 - 4), 2))),           textureRepeats, textureRepeats,//v29
        3.642f, topHeight, -0.5f * sqrtf(abs(diameter / 2.0 - pow(double(3.642 - 4), 2))),          textureRepeats, textureRepeats,//v30
        3.5f, topHeight, 0.0f,                                                                      textureRepeats, textureRepeats,//v31
    };

    // Index data to share position data
    //const int elementNumberIndices = 60;
    GLushort indices[] = {
        //bottom circle
        0,1,2,
        0,2,3,
        0,3,4,
        0,4,5,
        0,5,6,
        0,6,7,
        0,7,8,
        0,8,9,
        0,9,10,
        0,10,11,
        0,11,12,
        0,12,13,
        0,13,14,
        0,14,15,
        0,15,1,
        
        //top circle
        17,18,19,
        17,19,20,
        17,20,21,
        17,21,22,
        17,22,23,
        17,23,24,
        17,24,25,
        17,25,26,
        17,26,27,
        17,27,28,
        17,28,29,
        17,29,30,
        17,30,31,
        17,31,18,

        
        //half cylinder wall covering half-bottom
        1,18,2,
        3,19,2,
        3,20,4,
        5,21,4,
        5,22,6,
        7,23,6,
        7,24,8,
        9,25,8,
        9,26,10,
        11,27,10,
        11,28,12,
        13,29,12,
        13,30,14,
        15,31,14,

        //half cylinder wall covering half-top
        18,2,19,
        20,3,19,
        20,4,21,
        22,5,21,
        22,6,23,
        24,7,23,
        24,8,25,
        26,9,25,
        26,10,27,
        28,11,27,
        28,12,29,
        30,13,29,
        30,14,31,
        18,15,31

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex)));
    glEnableVertexAttribArray(1);
}

//function to generate water bottle body mesh object
void createWaterBottleMesh(GLMesh& mesh)
{
   
    const float baseHeight = -2.0f;
    const float topHeight = 3.0f;
    const float zCenter = 4.0;            //cylinder center point along z-axis
    const float xLowBound = 4.0;
    // Position and Color data 
    const float textureRepeats = 0.1;
    GLfloat verts[] = {

        //rough cylinder prototype
        // Vertex Positions                             // Texture Coordinates
        //0-16top
        zCenter, baseHeight, 0.0f,                      textureRepeats, textureRepeats,//v0         // bottom circle to cylinder
        xLowBound, baseHeight, 0.0,                             0.0, 0.0f,//v1
        xLowBound + .25f,baseHeight, -.25f,             0.0f, textureRepeats,//v2
        xLowBound + .5f, baseHeight, -.5f,               0.0f, 0.0f,//v3
        xLowBound + .75f,baseHeight,  -.75f,             0.0f, textureRepeats,//v4
        xLowBound + 1.0f, baseHeight, -1.0f,                 0.0f, 0.0f,//v5
        xLowBound + 1.25f,baseHeight,  -.75f,             0.0f, textureRepeats,//v6
        xLowBound + 1.5f, baseHeight,  -.5,               0.0f, 0.0f,//v7
        xLowBound + 1.75f, baseHeight, -.25,             0.0f, textureRepeats,//v8
                         
        xLowBound + 2.0f, baseHeight, 0.0f,                       0.0f, 0.0f,//v9
        xLowBound + 1.75f,baseHeight, .25f,                0.0f, textureRepeats,//v10
        xLowBound + 1.5f, baseHeight, .5f,                 0.0f, 0.0f,//v11
        xLowBound + 1.25f,baseHeight, .75f,               0.0f, textureRepeats, //v12
        xLowBound + 1.0f, baseHeight, 1.0f,                       0.0f, 0.0f,//v13
        xLowBound + .75f, baseHeight, .75f,             0.0f, textureRepeats,//v14
        xLowBound + .5f, baseHeight, .5,              0.0f, 0.0f,//v15
        xLowBound + .25f,baseHeight, .25,             0.0f, textureRepeats,//v16
         

        zCenter, topHeight, 0.0f,                          textureRepeats, textureRepeats,//v17     // top circle to cylinder
        xLowBound, topHeight, 0.0,                              0.0f, 0.0f,//v18
        xLowBound + .25f,topHeight, -.25f,                0.0f, textureRepeats,//v19
        xLowBound + .5f, topHeight, -.5f,                 0.0f, 0.0f,//v20
        xLowBound + .75f,topHeight,  -.75f,               0.0f, textureRepeats,//v21
        xLowBound + 1.0f, topHeight, -1.0f,              0.0f, 0.0f,//v22
        xLowBound + .75f,topHeight,   -.75f,              0.0f, textureRepeats,//v23
        xLowBound + .5f, topHeight,   -.5,                0.0f, 0.0f,//v24
        xLowBound + .25f, topHeight, -.25,               0.0f, textureRepeats,//v25
        
        xLowBound, topHeight, 0.0f,                           0.0f, 0.0f,//v26
        xLowBound + .25f,topHeight, .25f,               0.0f, textureRepeats,//v27
        xLowBound + .5f, topHeight, .5f,              0.0f, 0.0f,//v28
        xLowBound + .75f,topHeight, .75f,            0.0f, textureRepeats,//v29
        xLowBound + 1.0f, topHeight, 1.0f,          0.0f, 0.0f,//v30
        xLowBound + .75f,topHeight, .75f,           0.0f, textureRepeats,//v31
        xLowBound + .5f, topHeight, .5,             0.0f, 0.0f,//v32
        xLowBound + .25f,topHeight, .25,            0.0f, textureRepeats,//v33
    };

    // Index data to share position data
    //const int elementNumberIndices = 60;
    GLushort indices[] = {
        //bottom circle
        0,2,1,
        0,2,3,
        0,3,4,
        0,4,5,
        0,5,6,
        0,6,7,
        0,7,8,
        0,8,9,
        0,9,10,
        0,10,11,
        0,11,12,
        0,12,13,
        0,13,14,
        0,14,15,
        0,15,16,
        0,16,1,
        //top circle
        17,18,19,
        17,19,20,
        17,20,21,
        17,21,22,
        17,22,23,
        17,23,24,
        17,24,25,
        17,25,26,
        17,26,27,
        17,27,28,
        17,28,29,
        17,29,30,
        17,30,31,
        17,31,32,
        17,32,33,
        17,33,18,
        
        //half cylinder wall covering half-bottom
        1,18,2,
        3,19,2,
        3,20,4,
        5,21,4,
        5,22,6,
        7,23,6,
        7,24,8,
        9,25,8,
        9,26,10,
        11,27,10,
        11,28,12,
        13,29,12,
        13,30,14,
        15,31,14,
        15,32,16,
        1,33,16,
        //half cylinder wall covering half-top
        18,2,19,
        20,3,19,
        20,4,21,
        22,5,21,
        22,6,23,
        24,7,23,
        24,8,25,
        26,9,25,
        26,10,27,
        28,11,27,
        28,12,29,
        30,13,29,
        30,14,31,
        32,15,31,
        32,16,33,
        18,1,33
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex)));
    glEnableVertexAttribArray(1);
}

//Helmet Meshes
//function to generate red headlamp mesh object
void createRedHLMesh(GLMesh& mesh) {
    float t = 2.0;
    // Position and Color data 

    const float textureRepeats = 0.18f;
    GLfloat verts[] = {
        //head lamp boxes
       //red headlamp                            // Texture Coordinates
       -0.25f, 0.5f, t + 0.25f,                  0.0f, textureRepeats , // front left upper corner of head-lamp1 v0
       -0.25f, 0.0f, t + 0.25f,                  textureRepeats, textureRepeats, // front left lower corner of head-lamp1 v1
        0.25f, 0.0f, t + 0.25f,                  textureRepeats, 0.0f, // front right lower corner of head-lamp1 v2
        0.25f, 0.5f, t + 0.25f,                  0.0f, 0.0f, // front right upper corner of head-lamp1 v3

       //red head lamp back&sides
        -0.25f, 0.5f, 1.0f,                     0.0, textureRepeats, // back left upper corner of head-lamp1 v4
        -0.25f, 0.0f, 1.0f,                     0.0f, 0.0f, // back left lower corner of head-lamp1 v5
         0.25f, 0.0f, 1.0f,                     textureRepeats, 0.0f, // back right lower corner of head-lamp1 v7               -6    
        0.25f, 0.5f, 1.0f,                      textureRepeats, textureRepeats // back right upper corner of head-lamp1 v6      -7


    };


    GLushort indices[] = {
        //headlamp- red front face
         0, 1, 2,
         3, 2, 0,
        //headlamp -red 3d faces
        4, 5, 6,//
        7, 6, 4,//back face
        7, 6, 2,//
        3, 2, 7,//right face
        4, 5, 1,//
        0, 1, 4,// left face
        4, 7, 0,//
        0, 3, 7,//top face
        5, 6, 1,//
        1, 2, 6//bottom face
    };
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + 1)));
    glEnableVertexAttribArray(1);

}

// constructor function for the grey headlamp cube mesh
void createGreyHLMesh(GLMesh& mesh) {
    float t = 2.0;
    // Position and Color data 
    const float textureRepeat = 0.2f;
    GLfloat verts[] = {

        //grey headlamp                          // Texture Coordinates
       -0.3f, 0.0f, t + 0.26f,                   0.0, 0.0f, // front left upper corner of head-lamp2 v0
       -0.35f, -0.5f, t + 0.26f,                 0.0f, 0.0f,   // front left lower corner of head-lamp2 v1
        0.35f, -0.5f, t + 0.26f,                 0.0f, textureRepeat,   // front right lower corner of head-lamp2 v2
        0.3f, 0.0f, t + 0.26f,                   0.0f, textureRepeat,   // front right upper corner of head-lamp2 v3

       //grey headlamp back&sides
        - 0.3f, 0.0f, 1.0f,                    0.0f, 0.0f, // back left upper corner of head-lamp2 v4
       -0.35f, -0.5f, 1.0f,                    0.0f, textureRepeat,   // back left lower corner of head-lamp2 v5
        0.35f, -0.5f, 1.0f,                    0.0f, textureRepeat,   // back right lower corner of head-lamp2 v6
        0.3f, 0.0f, 1.0f,                      0.0f, 0.0f   // back right upper corner of head-lamp2 v7
    };

    GLushort indices[] = {
        //head-lamp front faces
        0, 1, 3,
        3, 2, 1,
        //grey 3d faces
        4, 5, 7,
        6, 7, 5, //back

        6, 7, 2,
        2, 3, 7,//right

        4, 5, 1,
        0, 1, 4,//left

        4, 7, 0,
        0, 3, 7,//top

        5, 6, 1,
        1, 2, 6 // bottom
    };
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV );

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + 1)));
    glEnableVertexAttribArray(1);
}

// The constructor function for the carpet plane mesh
void createCarpetMesh(GLMesh& mesh)
{
    // Position and Texture data
    const float textureRepeats = 40.0f;

    GLfloat verts[] = {

        //carpet floor coordinates           // Texture Coordinates
        -8.0f, -1.0f,  -8.0f,                0.0f, textureRepeats,     //back left corner of carpet v0
        8.0f,  -1.0f,  -8.0f,                textureRepeats, textureRepeats,     //back right corner of carpet v1
        -8.0f,  -1.0f,  8.0f,                0.0f, 0.0f,     //front left corner of carpet v2
        8.0f,  -1.0f,  8.0f,                 textureRepeats, 0.0f      //front right corner of carpet v3
    };

    // Index data to share position data
    //const int elementNumberIndices = 60;
    GLushort indices[] = {
        
        //carpet
        2, 3, 1,
        0, 2, 1
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + 1)));
    glEnableVertexAttribArray(1);
}

// The constructor function for the helmet polygon mesh
void createHelmetMesh(GLMesh& mesh)
{

    float t = 2.0; //helmet radius control variable
    // Position and Color data 
    const float textureRepeats = 1.0;
    const float constantMultiplier = 0.7;
    GLfloat verts[] = {

        //helmet prototype
        // Vertex Positions             // Texture Coordinates
         -1.0f,  t, 0.0f,               0.0f, 0.0f, // vertex 0
         1.0f, t, 0.0f,                 0.0f, textureRepeats, // vertex 1
         -t + 0.25f, -0.75, 0.0f,       textureRepeats, textureRepeats, // vertex 2
         t - 0.25f,  -0.75, 0.0f,       textureRepeats, textureRepeats, // vertex 3
         0.0f, -0.75f, t + 0.25f,       textureRepeats, 0.0f, // vertex 4
         0.0f,  1.0f,  t + 0.25f,       textureRepeats, textureRepeats, // vertex 5
         0.0f, -0.75f, -t + 0.25f,      0.0f, 0.0f, // vertex 6
         0.0f, 1.0f, -t + 0.25f,        0.0f, textureRepeats, // vertex 7
         t, 0.0f, -1.0f,                textureRepeats, 0.0f, // vertex 8
         t, 0.0f, 1.0f,                 0.0f, 0.0f, // vertex 9
         -t, 0.0f, -1.0f,               0.0f, 0.0f, // vertex 10
         -t, 0.0f, 1.0f,                0.0f, 0.0f // vertex 11 - front corner below headlamps

    };

    // Index data to share position data
    //const int elementNumberIndices = 60;
    GLushort indices[] = {
         //helmet
         0, 11, 5,
         0, 5, 1,
         0, 1, 7,
         0, 7, 10,
         0, 10, 11,
         1, 5, 9,
         5, 11, 4,
         11, 10, 2,
         10, 7, 6,
         7, 1, 8,
         3, 9, 4,
         3, 4, 2,
         3, 2, 6,
         3, 6, 8,
         3, 8, 9,
         4, 9, 5,
         2, 4, 11,
         6, 2, 10,
         8, 6, 7,
         9, 8, 1

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + 1)));
    glEnableVertexAttribArray(1);
}

/*
 * Program Generation Functions
 * ================================
 */

//destructor function for mesh objects
void destroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}

// Implements the UCreateShaders function
bool createShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[1012];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}

//destructor function for GL program
void destroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}

/*Generate and load the texture*/
bool createTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   //orginally: GL_LINEAR
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}

//destructor function for texture handles
void destroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}

/*
 * Utility Interface Functions
 * ===============================
 */

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.Position += camera.Up * camera.MovementSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.Position -= camera.Up * camera.MovementSpeed * deltaTime;
   /* if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (perspectDefault) {
            projection = glm::ortho(0.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, -1.0f, 3.0f);
            //glOrtho(-4.0, 4.0, -4.0, 4.0, 4.0, -4.0);

            perspectDefault = false;
        }
        else if(!perspectDefault) {
            projection = glm::perspective(70.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 1.0f, 1000.0f);
            camera.Front.x = 0.0;
            camera.Front.y = 0.0;
            camera.Front.z = 5.0;
            glm::mat4 view = camera.GetViewMatrix();
            perspectDefault = true;
        }
    }*/

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // determine mouse's location of orgin
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    //determine mouse's current location
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    //send mouse coordinates to camera object to process perspective shift
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double genericOffset, double speedOffset)
{
    camera.ProcessMouseScroll(speedOffset);
}