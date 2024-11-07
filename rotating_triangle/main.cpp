#include <glad/glad.h>
#include <GLFW/glfw3.h>
//glm: math library for matrix transformations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// to load shader files:
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

// to load shader files
std::string loadShaderSource(const char *filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;   
    buffer << file.rdbuf(); // reads the file into a string stream
    return buffer.str();    // returns the sahder code as a string
}

// adjusts theOpenGL viewport if the window is resized
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    // Initialize GLFW Library
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Window Hints: these set up OpenGL version 3.3 and the core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Rotating Triangle", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // makes the window's OpenGL context current, which allows OpenGL calls to affect this specific window
    glfwMakeContextCurrent(window);

    // framebuffer_size_callback handles window resizing
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //loads OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // soad shaders from files and stores them as const char* strings
    std::string vertexShaderCode = loadShaderSource("shaders/vertex_shader.glsl");
    std::string fragmentShaderCode = loadShaderSource("shaders/fragment_shader.glsl");
    const char *vertexShaderSource = vertexShaderCode.c_str();
    const char *fragmentShaderSource = fragmentShaderCode.c_str();

    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // new shader obj of type GL_VERTEX_SHADER
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);  // attaches the vertex shader src to shader obj
    glCompileShader(vertexShader);  

    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader Compilation Error: " << infoLog << std::endl;
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Check for compilation errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader Compilation Error: " << infoLog << std::endl;
    }

    // Link shaders to create the shader program
    GLuint shaderProgram = glCreateProgram();   //Creates a shader program to link both shaders together.
    glAttachShader(shaderProgram, vertexShader); //Attaches the vertex and fragment shaders to the program
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);  //Links the shaders into a single executable program.

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Error: " << infoLog << std::endl;
    }
    // Clean up shaders after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // defines the positions of a triangle’s vertices.
    float vertices[] = {
        0.0f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f};

    // create Vertex Array Object (VAO) and Vertex Buffer Object (VBO) to store vertex data
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO); 
    glGenBuffers(1, &VBO);  // Generate a buffer ID

    //Buffer Binding: Binds the VAO and VBO, then copies the vertex data to the VBO 
    glBindVertexArray(VAO);     // Bind the VAO first
    glBindBuffer(GL_ARRAY_BUFFER, VBO);     // Bind VBO while VAO is bound
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);      // Send data to GPU

    //Specifies how OpenGL should interpret the vertex data (3 floats per vertex) 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0); // Enables the attribute at location 0 (first attribute)
    
    float rotationAngle;
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT); //Clears the screen

        glUseProgram(shaderProgram); //Activates the shader program

        // Calculate transformation: a rotation matrix based on time
        glm::mat4 transform = glm::mat4(1.0f);  // 4x4 identity matrix
        
        rotationAngle = (float)glfwGetTime();

        //glm::rotate(matrixToTransform, rotation angle in rad, rotation axis vector (z axis) )
        float rotationAngleDegrees = rotationAngle * (180.0f / glm::pi<float>());
        transform = glm::rotate(transform, rotationAngle, glm::vec3(0.0f, 0.0f, 1.0f));

        std::cout << "Rotation Angle (radians): " << rotationAngle << std::endl;
        std::cout << "Rotation Angle (degrees): " << rotationAngleDegrees << std::endl;
        

        GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform"); 
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform)); // passes the transformation matrix to the shader

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3); //Draws the triangle

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();

    return 0;
}
