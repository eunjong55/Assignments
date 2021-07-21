#include <stdio.h>
#include <GL/glew.h> // for using extensions of opengl
#include <GL/glut.h> // for window
#include "LoadShaders.h" // for shading
#include <vector>
#include <time.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

GLuint active_vao = 0;
const GLuint num_of_models = 3;
const char* obj_filepath[num_of_models] =
{
    "house.obj",
    "sphere.obj",
    "teapot.obj"
};

std::vector<GLfloat> vertices[num_of_models]; // position of vertices
std::vector<GLuint> faces[num_of_models]; // information about vertices which constitute the faces

GLuint vao[num_of_models]; // VAO
GLuint vbo[num_of_models][2]; // VBO
GLuint program;

bool load_obj(std::vector<GLfloat>& vertices, std::vector<GLuint>& faces, const char* filepath);
void init();
void display();
void bind_buffer(GLint buffer, std::vector<GLfloat>& vec, int program, const GLchar* attri_name, GLint attri_size);
void bind_buffer(GLint buffer, std::vector<GLuint>& vec, int program);
int build_program();
void keyboard(unsigned char key, int x, int y);

void main(int argc, char** argv)
{
    glutInit(&argc, argv); // initialize the glut to make window
    glutInitDisplayMode(GLUT_RGBA); // select color model, number of buffers, type of buffer
    glutInitWindowSize(512, 512); // select window size
    glutCreateWindow(argv[0]); // create window, input : window name
    GLenum err = glewInit(); // initialize the GLEW to load opengl extension

    if (err != GLEW_OK) { // error handler
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    init(); // call the function init()
    glutDisplayFunc(display);// register call back function for display event
    glutKeyboardFunc(keyboard); // enable keyboard function
    glutMainLoop(); // enter the display event loop
}

bool load_obj(std::vector<GLfloat>& vertices, std::vector<GLuint>& faces, const char* filepath)
{
    ifstream file(filepath);
    string str;

    if (!file.is_open())
    {
        printf("file is not opened\n");
        return false;
    }

    while (getline(file, str))
    {
        stringstream ss(str); // for getting string token
        string buf;
        ss >> buf;
        if (buf == "v")
        {
            while (ss >> buf) vertices.push_back(stof(buf)); // change string to float, and store the data to the vector
        }
        else if (buf == "f")
        {
            while (ss >> buf) faces.push_back(stoi(buf)-1);  // change string to int, and store the data to the vector
        }
    }
    return true;
}

void init()
{
    srand(clock());
    program = build_program(); // build program
    for (int i = 0; i < num_of_models; ++i)
    {
        load_obj(vertices[i], faces[i], obj_filepath[i]); // load obj files

        glGenVertexArrays(1, &vao[i]); // generate VAOs
        glBindVertexArray(vao[i]); // bind vao to server
        glGenBuffers(2, vbo[i]); // generate VBOs
        bind_buffer(vbo[i][0], vertices[i], program, "vPosition", 3); // set vbo type and store faces data to vbo
        bind_buffer(vbo[i][1], faces[i], program); // set vbo type and store faces data to vbo
    }

    glEnable(GL_DEPTH_TEST); // for depth testing
    glDepthFunc(GL_LESS); // set depth test function

    glLineWidth(1.0f); // set line thickness
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // change view mode to polygon mode

    glEnable(GL_CULL_FACE); // for enable face culling
    glCullFace(GL_BACK); // for enable back face culling
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the buffer
    glBindVertexArray(vao[active_vao]); // bind the vao about active_vao to server

    GLfloat theta = 0.001f * clock(); // The theta variable changes linearly over time.
    using namespace glm; // to use glsl
    mat4 T(1.0f); // declare identity matrix
    T = rotate(T, theta, vec3(0.0f, 1.0f, 0.0f)); // rotate the box around the y-axis
    GLint location = glGetUniformLocation(program, "T"); // find the location of T variable in program
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(T)); // change the value of uniform variable in hw4.vert to T matrix

    glDrawElements(GL_TRIANGLES, faces[active_vao].size(), GL_UNSIGNED_INT, (void*)0); // draw object with elements information
    glFlush(); // 
    glutPostRedisplay(); // 
}

void bind_buffer(GLint buffer, std::vector<GLfloat>& vec, int program, const GLchar* attri_name, GLint attri_size)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer); // bind buffer to GL_ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vec.size(), vec.data(), GL_STATIC_DRAW); // allocate data to buffer.
    GLint location = glGetAttribLocation(program, attri_name); // get location of attri_name in compiled program
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0); // generate vertex attribute pointer
    glEnableVertexAttribArray(location);  // enable vertex attribute array
}
void bind_buffer(GLint buffer, std::vector<GLuint>& vec, int program)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer); //  bind buffer to GL_ELEMENT_ARRAY_BUFFER
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * vec.size(), vec.data(), GL_STATIC_DRAW); // allocate data to buffer.
}

int build_program()
{
    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER, "hw5.vert" },
        { GL_FRAGMENT_SHADER, "hw5.frag" },
        { GL_NONE, NULL }
    };
    GLuint program = LoadShaders(shaders); // compile the shaders
    glUseProgram(program);
    return program;
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '1': active_vao = 0; glutPostRedisplay(); break; // redraw display.
    case '2': active_vao = 1; glutPostRedisplay(); break;
    case '3': active_vao = 2; glutPostRedisplay(); break;
    }
}
