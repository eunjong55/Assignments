#include <stdio.h>
#include <GL/glew.h> // for using extensions of opengl
#include <GL/glut.h> // for window
#include "LoadShaders.h" // for shade.
#include <vector>
#include <time.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef std::vector<GLfloat> GLvec;

GLuint vao, vbo[2];
GLvec vertices, colors;
GLuint program;
GLfloat scaling_factor = 1.0f;
GLuint drawing_mode = 0;


void init();
void display();
void get_box_3d(std::vector<GLfloat>& p, GLfloat lx, GLfloat ly, GLfloat lz);
void get_box_random_color(std::vector<GLfloat>& color);
void bind_buffer(GLint buffer, GLvec& vec, int program, const GLchar* attri_name, GLint attri_size);
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
    glutKeyboardFunc(keyboard);
    glutMainLoop(); // enter the display event loop
}

void init()
{
    srand(clock());
    program = build_program(); // build a program
    get_box_3d(vertices, 1, 1, 1); // get cube positions
    get_box_random_color(colors); // get cube face colors

    glGenVertexArrays(1, &vao); // generate VAO
    glBindVertexArray(vao); // use vao
    glGenBuffers(2, vbo);  // generate VBOs
    bind_buffer(vbo[0], vertices, program, "vPosition", 3); // assign vertice positions to "vPosition" variable which is in the program
    bind_buffer(vbo[1], colors, program, "vColor", 3);  // assign vertice positions to "vColor" variable which is in the program

    glEnable(GL_DEPTH_TEST);  // enable depth testing
    glDepthFunc(GL_LESS); // choose depth testing function
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear buffer which is used to previous display 
    glBindVertexArray(vao); // use vao

    GLint location;
    GLfloat theta = 0.001f * clock(); // theta is linearly increasd
    location = glGetUniformLocation(program, "drawing_mode"); // find the location of "drawing_mode" in program
    glUniform1i(location, drawing_mode); // change the value of uniform variable in hw4.frag to drawing_mode value

    using namespace glm; // to use glsl
    mat4 T(1.0f);
    T = rotate(T, theta, vec3(1.0f, 0.0f, 0.0f)); // rotate the box around the x-axis
    T = rotate(T, theta, vec3(0.0f, 0.1f, 0.0f)); // rotate the box around the y-axis
    T = rotate(T, theta, vec3(1.0f, 0.0f, 0.0f)); // rotate the box around the x-axis
    T = scale(T, vec3(scaling_factor)); // set scale

    location = glGetUniformLocation(program, "T"); // find the location of T variable in program
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(T)); // change the value of uniform variable in hw4.vert to T matrix

    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3); // draw cube
    glFlush(); // apply the changes immediately
    glutPostRedisplay(); // redraw display.
}

void get_box_3d(std::vector<GLfloat>& p, GLfloat lx, GLfloat ly, GLfloat lz) // get box positions
{
    static const GLfloat box_vertices[] =
    {
         0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f, 0.5f,-0.5f,
         0.5f, 0.5f,-0.5f,  0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f,-0.5f,-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f,
         0.5f,-0.5f, 0.5f, -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,
         0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f, -0.5f,-0.5f,-0.5f,
        -0.5f, 0.5f, 0.5f, -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,
         0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  0.5f,-0.5f, 0.5f,
         0.5f, 0.5f, 0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
         0.5f,-0.5f,-0.5f,  0.5f, 0.5f, 0.5f,  0.5f,-0.5f, 0.5f,
         0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f,
         0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f, -0.5f, 0.5f, 0.5f,
    };

    p.resize(sizeof(box_vertices) / sizeof(GLfloat));
    memcpy(p.data(), box_vertices, sizeof(box_vertices));
    size_t n = p.size() / 3;
    for (int i = 0; i < n; ++i)
    {
        p[3 * i + 0] *= lx;
        p[3 * i + 1] *= ly;
        p[3 * i + 2] *= lz;
    }
}

void get_box_random_color(std::vector<GLfloat>& color) // get box colors
{
    color.resize(108);

    GLfloat* p = color.data();
    for (size_t i = 0; i < 6; ++i)
    {
        GLfloat side_color[3] =
        {
            0.5f * rand() / RAND_MAX + 0.5f,
            0.5f * rand() / RAND_MAX + 0.5f,
            0.5f * rand() / RAND_MAX + 0.5f
        };
        for (size_t j = 0; j < 6; ++j)
        {
            *p++ = side_color[0];
            *p++ = side_color[1];
            *p++ = side_color[2];
        }
    }
}

void bind_buffer(GLint buffer, GLvec& vec, int program, const GLchar* attri_name, GLint attri_size)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer); // bind buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vec.size(), vec.data(), GL_STATIC_DRAW); // allocate data.
    GLint location = glGetAttribLocation(program, attri_name); // get location of attri_name in compiled program
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0); 
    glEnableVertexAttribArray(location);  // enable vertex attribute array
}

int build_program()
{
    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER, "hw4.vert" },
        { GL_FRAGMENT_SHADER, "hw4.frag" },
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
    case '1': drawing_mode = 0; glutPostRedisplay(); break; // redraw display.
    case '2': drawing_mode = 1; glutPostRedisplay(); break;
    case '3': drawing_mode = 2; glutPostRedisplay(); break;
    case '+': scaling_factor *= 1.1 ; glutPostRedisplay(); break;
    case '-': scaling_factor *= 0.9; glutPostRedisplay(); break;
    }
}
