
#include <stdio.h>
#include <GL/glew.h> // for using extensions of opengl
#include <GL/glut.h> // for window
#include "LoadShaders.h" // for shade.
#include <glm/gtc/matrix_transform.hpp> // for use the scale(), rotate()
#include <glm/gtc/type_ptr.hpp> // for returns the pointer of a given vector or matrix.
#include <glm/glm.hpp>
#include <time.h>


const GLsizei NumVertices = 6;
GLfloat vertices[NumVertices][2] = { // positions of vertices [-1 ~ 1].
   {-0.90f, -0.90f}, {0.85f, -0.90f},
   {-0.90f, 0.85}, {0.90f, -0.85f},
   {0.90f, 0.90f}, {-0.85f, 0.90f}
};

GLuint program;
GLuint Buffers[1];
GLuint VertexArrays[1];

void init();
void display();

void main(int argc, char** argv)
{
    glutInit(&argc, argv); // initialize the glut to make window.
    glutInitDisplayMode(GLUT_RGBA); // select color model, number of buffers, type of buffer.
    glutInitWindowSize(512, 512); // select window size
    glutCreateWindow(argv[0]); // create window, input : window name.
    GLenum err = glewInit(); // initialize the GLEW to load opengl extension.

    if (err != GLEW_OK) { // error handler
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    init(); // call the function init()
    glutDisplayFunc(display);// register call back function for display event
    glutMainLoop(); // enter the display event loop
}

void init()
{
    glGenVertexArrays(1, VertexArrays); //Generate vertex array objects (VAOs)
    glBindVertexArray(VertexArrays[0]); //specify the current active VAO.

    glGenBuffers(1, Buffers); //create buffer
    glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]); // select type for buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // momory allocation and stor the vertex data to buffer

    ShaderInfo shaders[] = { // to process shade.
    {GL_VERTEX_SHADER, "hw3.vert"}, // for vertex shader (position)
    {GL_FRAGMENT_SHADER, "hw3.frag"}, // for fragment shader (color)
    {GL_NONE, NULL}
    };

    program = LoadShaders(shaders); // compile shader and create program object.
    glUseProgram(program); // activate program object.

    GLint location = glGetAttribLocation(program, "vPosition"); //Finds the location of a vPosition attribute. 
    glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 0, 0); // Specifies how to read the buffer data through the attribute
    glEnableVertexAttribArray(location); // Enable the attribute.
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT); // clear the buffer
    glBindVertexArray(VertexArrays[0]); //Specifies the current active VAO.

    GLfloat theta = 0.001f * clock(); // It changes linearly over time.
    using namespace glm;
    mat4 T(1.0f); // create identity matrix
    T = rotate(T, theta, vec3(-1.0f, 1.0f, 0.0f)); // rotate
    T = scale(T, vec3(0.5f)); // scale change.
    GLuint location = glGetUniformLocation(program, "T"); // return the ID number of uniform variable "T" declared in hw3.vert
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(T)); // Assign a T matrix to uniform variable in hw3.vert

    glDrawArrays(GL_TRIANGLES, 0, NumVertices); // draw command.
    glFlush(); // Forces the execution of OpenGL commands in finite time.

    glutPostRedisplay(); // redraw window.
}

