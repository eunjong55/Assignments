#include <stdio.h>
#include <GL/glew.h> // for using extensions of opengl
#include <GL/glut.h> // for using window
#include "LoadShaders.h" // to compile shader and to link compiled shaders with program
#include <vector>
#include <time.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define _USE_MATH_DEFINES
#include <math.h> // for using M_PI


typedef std::vector<GLfloat> GLvec;

GLuint vao, vbo[2]; // for VAO and VBO
GLvec vertices, colors; 
GLuint program;
bool show_vertices = FALSE;
bool show_wireframe = FALSE;

void init(); // to get vertex position and vertex color
void display(); // draw vertices
void bind_buffer(GLint buffer, GLvec& vec, int program, const GLchar* attri_name, GLint attri_size); // assign data to attri_name
int build_program(); // build program
void keyboard(unsigned char key, int x, int y); // keyboard event function
void get_color_3d_by_pos(GLvec& c, GLvec& p, GLfloat offset); // to get vertex color
void get_sphere_3d(GLvec& p, GLfloat r, GLint subh, GLint suba); // to get vertex position
void draw_sphere(const GLfloat* trans_mat); // to draw sphere


void main(int argc, char** argv)
{
    glutInit(&argc, argv); // initialize the glut to make window
    glutInitDisplayMode(GLUT_RGBA); // select color model
    glutInitWindowSize(512, 512); // select window size
    glutCreateWindow(argv[0]); // create window

    GLenum err = glewInit(); // initialize the GLEW to load opengl extension
    if (err != GLEW_OK) { // error handler
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    init(); // call the function init()
    glutDisplayFunc(display); // register call back function for the display event
    glutKeyboardFunc(keyboard); // register call back function for the keyboard event
    glutMainLoop(); // enter the display event loop
}

void init()
{
    srand(clock()); 
    program = build_program(); // build a program
    get_sphere_3d(vertices, 1, 20, 20); // get sphere positions
    get_color_3d_by_pos(colors, vertices, 0); // get colors

    glGenVertexArrays(1, &vao); // generate VAO
    glBindVertexArray(vao); // bind vao
    glGenBuffers(2, vbo);  // generate VBOs
    bind_buffer(vbo[0], vertices, program, "vPosition", 3); // assign vertice position data to "vPosition" variable which is in the program
    bind_buffer(vbo[1], colors, program, "vColor", 3);  // assign vertice color data to "vColor" variable which is in the program

    glEnable(GL_DEPTH_TEST); // for depth testing
    glDepthFunc(GL_LESS); // set depth test function

    glEnable(GL_CULL_FACE); // for enable face culling
    glCullFace(GL_BACK); // for enable back face culling
}

void display()
{
    GLint location = glGetUniformLocation(program, "T"); // find the location of T variable(uniform variable) in program
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear buffer
    glBindVertexArray(vao); // bind vao

    GLfloat theta = 0.001f * clock(); // theta is linearly increasd

    using namespace glm; // to use glsl
    mat4 T_earth(1.0f); // for earth position transformation
    mat4 T_sun(1.0f); // for sun position transformation
    mat4 T_moon(1.0f); // for moon position transformation

    T_earth = rotate(T_earth, theta, vec3(0.0f, 1.0f, 0.0f)); // self-rotaition
    T_earth = translate(T_earth, vec3(0.6f, 0.0f, 0.0f)); // translation to the earth position
    T_earth = rotate(T_earth, theta, vec3(0.0f, 1.0f, 0.0f)); // orbiting around the sun
    T_earth = scale(T_earth, vec3(0.1f)); // scaling

    T_sun = rotate(T_sun, theta, vec3(0.0f, 1.0f, 0.0f)); // sel-rotation
    T_sun = scale(T_sun, vec3(0.3f)); // scaling

    T_moon = rotate(T_moon, theta, vec3(0.0f, 1.0f, 0.0f)); // self-rotation
    T_moon = translate(T_moon, vec3(0.6f, 0.0f, 0.0f)); // translation to the earth position
    T_moon = rotate(T_moon, theta, vec3(0.0f, 1.0f, 0.0f)); // orbiting around the sun
    T_moon = translate(T_moon, vec3(0.2f, 0.0f, 0.0f)); // translation to the moon position
    T_moon = rotate(T_moon, theta, vec3(0.0f, 1.0f, 0.0f)); // orbitin around the earth
    T_moon = scale(T_moon, vec3(0.05f)); // scaling

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // fill the faces by color
    glEnable(GL_POLYGON_OFFSET_FILL); // when the polygons have same z-value, we can set priority
    glPolygonOffset(1, 1); // set offset

    glUniform1i(2, 0); // to set uniform variable where the location is 2 
    draw_sphere(value_ptr(T_sun)); // draw sun
    draw_sphere(value_ptr(T_earth)); // draw earth
    draw_sphere(value_ptr(T_moon)); // draw moon

    glDisable(GL_POLYGON_OFFSET_FILL); // disable offset fill
    
    if (show_vertices)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); // to draw vertices by point
        glPointSize(2); // to set font size
        glUniform1i(2, 1); // to set uniform variable where the location is 2 
        draw_sphere(value_ptr(T_sun)); // draw sun
        draw_sphere(value_ptr(T_earth)); // draw earth
        draw_sphere(value_ptr(T_moon)); // draw moon
    }

    if (show_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // to draw vertices by line
        glLineWidth(1); // to set line width

        glUniform1i(2, 1); // to set uniform variable where the location is 2
        draw_sphere(value_ptr(T_sun)); // draw sun
        draw_sphere(value_ptr(T_earth)); // draw earth
        draw_sphere(value_ptr(T_moon)); // draw moon
    }

    glFlush(); // apply the changes immediately
    glutPostRedisplay(); // redraw images.
}


void bind_buffer(GLint buffer, GLvec& vec, int program, const GLchar* attri_name, GLint attri_size)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer); // bind buffer with GL_ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vec.size(), vec.data(), GL_STATIC_DRAW); // allocate data to GL_ARRAY_BUFFER.
    GLint location = glGetAttribLocation(program, attri_name); // get location of attri_name in compiled program
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0); // specifies how to read the buffer data through the attribute
    glEnableVertexAttribArray(location);  // enable vertex attribute array
}

int build_program()
{
    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER, "hw6.vert" },
        { GL_FRAGMENT_SHADER, "hw6.frag" },
        { GL_NONE, NULL }
    };
    GLuint program = LoadShaders(shaders); // compile shaders and link program with compiled shaders 
    glUseProgram(program); // to use program
    return program;
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w': show_wireframe = !show_wireframe; show_vertices = FALSE; glutPostRedisplay(); break; // redraw display.
    case 'v': show_wireframe = FALSE; show_vertices = !show_vertices; glutPostRedisplay(); break; // redraw display.
    }
}

#define FPUSH_VTX3(p, vx, vy, vz) \
do { \
    p.push_back(vx); \
    p.push_back(vy); \
    p.push_back(vz); \
} while(0)

#define FSET_VTX3(vx, vy, vz, valx, valy, valz) \
do \
{ \
    vx=(float)(valx); \
    vy = (float)(valy);\
    vz = (float)(valz); \
} while (0)

void get_sphere_3d(GLvec& p, GLfloat r, GLint subh, GLint suba) // // to get sphere vertices position
{
    for (int i = 1; i <= subh; ++i)
    {
        double theta0 = M_PI * (i-1) / subh; // get angle of i-1
        double theta1 = M_PI * i / subh;

        double y0 = r * cos(theta0); 
        double rst0 = r * sin(theta0);
        double y1 = r * cos(theta1);
        double rst1 = r * sin(theta1);

        for (int j = 1; j <= suba; ++j)
        {
            double phi0 = 2 * M_PI * (j-1) / suba;
            double phi1 = 2 * M_PI * j / suba;

            double cp0 = cos(phi0);
            double sp0 = sin(phi0);
            double cp1 = cos(phi1);
            double sp1 = sin(phi1);

            float vx0, vy0, vz0, vx1, vy1, vz1;
            float vx2, vy2, vz2, vx3, vy3, vz3;

            FSET_VTX3(vx0, vy0, vz0, sp0 * rst0, y0, cp0 * rst0);
            FSET_VTX3(vx1, vy1, vz1, sp0 * rst1, y1, cp0 * rst1);
            FSET_VTX3(vx2, vy2, vz2, sp1 * rst0, y0, cp1 * rst0);
            FSET_VTX3(vx3, vy3, vz3, sp1 * rst1, y1, cp1 * rst1);

            if (i < subh)
            {
                FPUSH_VTX3(p, vx0, vy0, vz0);
                FPUSH_VTX3(p, vx1, vy1, vz1);
                FPUSH_VTX3(p, vx3, vy3, vz3);
            }

            if (1 < i)
            {
                FPUSH_VTX3(p, vx3, vy3, vz3);
                FPUSH_VTX3(p, vx2, vy2, vz2);
                FPUSH_VTX3(p, vx0, vy0, vz0);
            }
        }
    }
}

void get_color_3d_by_pos(GLvec& c, GLvec& p, GLfloat offset) // to get vertices color
{
    GLfloat max_val[3] = { -INFINITY, -INFINITY, -INFINITY };
    GLfloat min_val[3] = { INFINITY, INFINITY, INFINITY };

    int n = (int)(p.size() / 3);
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            GLfloat val = p[i * 3 + j];
            if (max_val[j] < val) max_val[j] = val;
            else if (min_val[j] > val) min_val[j] = val;
        }
    }

    GLfloat width[3] = {
        max_val[0] - min_val[0],
        max_val[1] - min_val[1],
        max_val[2] - min_val[2]
    };

    c.resize(p.size());
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            int k = i * 3 + j;
            c[k] = std::fminf((p[k] - min_val[j]) / width[j] + offset, 1.0f);
        }
    }
}

void draw_sphere(const GLfloat* trans_mat)
{
    glUniformMatrix4fv(1, 1, GL_FALSE, trans_mat); // to set uniform variable where the location is 2 
    glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // draw sphere
}
