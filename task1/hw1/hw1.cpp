// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>

int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "HW1 Dinamic Pyramid", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    //
    // color red 0.435294 green 0.258824 blue 0.258824
    // color red 0.917647 green 0.678431 blue 0.917647
    glClearColor( 0.917647, 0.678431, 0.917647, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader" );

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    glm::mat4 View       = glm::lookAt(
            glm::vec3(3,3,3), // Camera is at (4,3,-3), in World Space
            glm::vec3(0,0,0), // and looks at the origin
            glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
    );
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model      = glm::mat4(1.0f);

    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around

    // Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
    // A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
    //тут перечислены грани
    static const GLfloat g_vertex_buffer_data[] = {
            0.0f, 1.0f, 0.0f,
            -1.0f,-1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,

            0.0f, 1.0f, 0.0f,
            0.0f, -1.0f, -1.0f,
            1.0f,-1.0f, 1.0f,


            0.0f, 1.0f, 0.0f,
            -1.0f,-1.0f, 1.0f,
            0.0f, -1.0f, -1.0f,

            0.0f, -1.0f, -1.0f,
            -1.0f,-1.0f, 1.0f,
            1.0f, -1.0f, 1.0f

    };
    // цвета всех вершин
    /*
     * Some nice colors:
#declare Maroon = color red 0.556863 green 0.137255 blue 0.419608
#declare MediumAquamarine = color red 0.196078 green 0.8 blue 0.6
#declare MediumBlue = color red 0.196078 green 0.196078 blue 0.8
#declare MediumForestGreen = color red 0.419608 green 0.556863 blue 0.137255
#declare MediumGoldenrod = color red 0.917647 green 0.917647 blue 0.678431
#declare MediumOrchid = color red 0.576471 green 0.439216 blue 0.858824
#declare MediumSeaGreen = color red 0.258824 green 0.435294 blue 0.258824
#declare MediumSlateBlue = color red 0.498039 blue 1.0
#declare MediumSpringGreen = color red 0.498039 green 1.0
#declare MediumTurquoise = color red 0.439216 green 0.858824 blue 0.858824
#declare MediumVioletRed = color red 0.858824 green 0.439216 blue 0.576471
#declare MidnightBlue = color red 0.184314 green 0.184314 blue 0.309804
#declare Navy = color red 0.137255 green 0.137255 blue 0.556863
#declare NavyBlue = color red 0.137255 green 0.137255 blue 0.556863
#declare Orange = color red 1 green 0.5 blue 0.0
#declare OrangeRed = color red 1.0 green 0.25
#declare Orchid = color red 0.858824 green 0.439216 blue 0.858824
#declare Gold = color red 0.8 green 0.498039 blue 0.196078
     * */
    static const GLfloat g_color_buffer_data[] = {
            0.858824f, 0.439216f,  0.858824f,
            0.137255, 0.137255, 0.556863,
            0.196078f, 0.8f, 0.6f,

            0.858824f, 0.439216f,  0.858824f,
            0.439216, 0.858824, 0.858824,
            0.196078f, 0.8f, 0.6f,

            0.858824f, 0.439216f,  0.858824f,
            0.137255, 0.137255, 0.556863,
            0.439216, 0.858824, 0.858824,

            0.439216, 0.858824, 0.858824,
            0.137255, 0.137255, 0.556863,
            0.196078f, 0.8f, 0.6f
    };

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    GLuint colorbuffer;
    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

    float x_ = -3;
    float y_ = 0;
    float r2_ = 9;
    int x_coord_iter = 0;
    int max_iter = 600;
    double x_pos = -3;
    double y_pos = 0;
    double alpha_ = 0;
    double radius_ = 3;
    double pi = 2* acos(0.0);


    bool pos_rotation = true;

    do{

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        View       = glm::lookAt(
                glm::vec3(x_pos, y_pos, 3), // Camera is at (4,3,-3), in World Space
                glm::vec3(0,0,0), // and looks at the origin
                glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
        );

        // Our ModelViewProjection : multiplication of our 3 matrices
        MVP  = Projection * View * Model; // Remember, matrix multiplication is the other way around

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
        );

        // 2nd attribute buffer : colors
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glVertexAttribPointer(
                1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
                3,                                // size
                GL_FLOAT,                         // type
                GL_FALSE,                         // normalized?
                0,                                // stride
                (void*)0                          // array buffer offset
        );

        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();


       // x_coord_iter++;
       alpha_ += 0.001;
       x_pos = radius_ * cos(alpha_);
       y_pos = radius_ * sin(alpha_);


        if(pos_rotation) {
            x_ += 0.05;

            //x_pos = -3 + (6 * x_coord_iter)/(max_iter*1.0);
            y_ = sqrt(r2_ - x_ * x_);
            //y_pos = sqrt(r2_ - x_pos * x_pos);
        }
        else{
            //x_pos = 3 - (6 * x_coord_iter)/(max_iter*1.0);
            //y_pos =-1 *  sqrt(r2_ - x_pos * x_pos);
            x_ -= 0.05;
            y_ = -1* sqrt(r2_ - x_ * x_);
        }
        /*if(x_coord_iter == max_iter) {
            pos_rotation = !pos_rotation;
            x_coord_iter = 0;
        }*/

        if(x_ >= 3) {
            pos_rotation = !pos_rotation;
           // x_ = 3;
        }
        if( x_ <=-3) {
            pos_rotation = !pos_rotation;
            //x_ = -3;
        }



    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &colorbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


