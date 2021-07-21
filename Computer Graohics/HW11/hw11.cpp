#include <stdio.h>
#include <GL/glew.h> // for using extensions of opengl
#include <GL/glut.h> // for using window
#include <GL/freeglut.h>
#include "LoadShaders.h" // to compile shader and to link compiled shaders with program
#include <vector>
#include <time.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define _USE_MATH_DEFINES
#include <math.h> // for using M_PI
#include "loadobj.h"

int shading_mode = 0;

// to assign values to uniform variables
#define UVAR(name, i) glGetUniformLocation(program[i], name)
#define UVARS(name) UVAR(name, shading_mode)

// to load model data
std::vector<tinyobj::real_t> vertices;
std::vector<tinyobj::real_t> normals;
std::vector<tinyobj::real_t> colors;
std::vector<std::vector<size_t>> vertex_map;
std::vector<std::vector<size_t>> material_map;
std::vector<tinyobj::shape_t> shapes;
std::vector<tinyobj::material_t> materials;
bool is_obj_valid = false;

typedef std::vector<GLfloat> GLvec;

void init(); // to get vertex position and vertex color
void display(); // draw vertices 
void bind_buffer(GLint buffer, GLvec& vec, int program, const GLchar* attri_name, GLint attri_size); // assign data to attri_name
void bind_buffer(GLint buffer, int program, const GLchar* attri_name, GLint attri_size); // assign data to attri_name

glm::mat4 parallel(double r, double aspect, double n, double f);

void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void mouse_wheel(int wheel, int dir, int x, int y);
void cb_special(int key, int x, int y);
void cb_menu(int value);
void keyboard(unsigned char key, int x, int y); // keyboard event function

void get_grid(std::vector<GLfloat>& p, GLfloat w, GLfloat h, int m, int n);
void make_grid(GLfloat w, GLfloat h, int m, int n);
void draw_grid(const GLfloat* trans_mat); 


struct Camera
{
    enum { ORTHOGRAPHIC, PERSPECTIVE };
    glm::vec3 eye;
    glm::vec3 center;
    glm::vec3 up;
    float zoom_factor;
    int projection_mode;
    float z_near;
    float z_far;
    float fovy;
    float x_right;

    Camera() :
        eye(0, 0, 8),
        center(0, 0, 0),
        up(0, 1, 0),
        zoom_factor(1.0f),
        projection_mode(ORTHOGRAPHIC),
        z_near(0.01f),
        z_far(100.0f),
        fovy((float)(M_PI / 180 * (30.0))),
        x_right(1.2f)
    {}
    glm::mat4 get_viewing() { return glm::lookAt(eye, center, up); }
    glm::mat4 get_projection(float aspect)
    {
        glm::mat4 P;
        switch (projection_mode)
        {
        case ORTHOGRAPHIC:
            P = parallel(zoom_factor * x_right, aspect, z_near, z_far);
            break;
        case PERSPECTIVE:
            P = glm::perspective(zoom_factor * fovy, aspect, z_near, z_far);
            break;
        }
        return P;
    }
};

Camera camera;

bool show_wireframe = FALSE;
bool show_grid = FALSE;
bool show_axis = FALSE;

glm::mat4 T(1.0f); // for object rotation
glm::mat4 TL(1.0f); 

glm::mat4 TR(1.0f); 

GLuint program[2];
GLuint vao[2];
GLuint vbo[2];
GLuint grid_vao, grid_vbo;
GLvec grid_vtx_pos;

int obj_mode;

GLuint active_vao = 0;

int button_pressed[3] = { GLUT_UP, GLUT_UP, GLUT_UP };
int mouse_pos[2] = { 0, 0 };
int colorMode = 0;

void draw_obj_model(int color_mode)
{
    glUniform1i(UVARS("ColorMode"), color_mode);
    glBindVertexArray(vao[shading_mode]);

    using namespace glm;
    using namespace tinyobj;

    for (size_t i = 0; i < shapes.size(); ++i)
    {
        for (size_t j = 0; j < material_map[i].size(); ++j)
        {
            // Set material properties.
            int m_id = material_map[i][j];
            if (m_id < 0) {
                glUniform1f(UVARS("n"), 10.0f);
                glUniform3f(UVARS("Ka"), 0.3f, 0.3f, 0.3f);
                glUniform3f(UVARS("Kd"), 1.0f, 1.0f, 1.0f);
                glUniform3f(UVARS("Ks"), 0.8f, 0.8f, 0.8f);
            }
            else {
                glUniform1f(UVARS("n"), materials[m_id].shininess);
                glUniform3fv(UVARS("Ka"), 1, materials[m_id].ambient);
                glUniform3fv(UVARS("Kd"), 1, materials[m_id].diffuse);
                glUniform3fv(UVARS("Ks"), 1, materials[m_id].specular);
            }

            glDrawArrays(GL_TRIANGLES, vertex_map[i][j], vertex_map[i][j + 1] - vertex_map[i][j]);
        }
    }
}

void main(int argc, char** argv)
{
    printf("Select the model (0 : Sports car, 1 : bunny) : ");
    scanf_s("%d", &obj_mode);
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

    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard); // register call back function for the keyboard event
    glutMouseWheelFunc(mouse_wheel);
    glutSpecialFunc(cb_special);

    glutMainLoop(); // enter the display event loop
}

void init()
{
    //srand(clock()); // to use random function
    ShaderInfo shaders[2][3] = {
        {
            { GL_VERTEX_SHADER, "phong.vert" },
            { GL_FRAGMENT_SHADER, "phong.frag" },
            { GL_NONE, NULL }
        },
        {
            { GL_VERTEX_SHADER, "gouraud.vert" },
            { GL_FRAGMENT_SHADER, "gouraud.frag" },
            { GL_NONE, NULL }
        }
    };

    for (int i = 0; i < 2; ++i)
        program[i] = LoadShaders(shaders[i]);

    tinyobj::attrib_t attrib;
    if (obj_mode == 0) {
        is_obj_valid = load_obj("models/sportsCar.obj", "models/", vertices, normals, vertex_map, material_map, attrib, shapes, materials);
        colors.resize(vertices.size());
    }
    else
    {
        is_obj_valid = load_obj("models/bunny.obj", "models/",
            vertices, normals, vertex_map, material_map, attrib, shapes, materials);
        colors.resize(vertices.size());
    }

    glGenVertexArrays(1, &vao[0]);
    glGenBuffers(2, vbo);
    // Bind vertex buffer objects.
    glBindVertexArray(vao[0]);
    bind_buffer(vbo[0], vertices, program[0], "vPosition", 3);
    bind_buffer(vbo[1], normals, program[0], "vNormal", 3);

    glBindVertexArray(vao[1]);
    bind_buffer(vbo[0], program[1], "vPosition", 3);
    bind_buffer(vbo[1], program[1], "vNormal", 3);


    make_grid(3, 3, 10, 10);

    glEnable(GL_DEPTH_TEST); // for depth testing
    glDepthFunc(GL_LESS); // set depth test function

    int menu_id = glutCreateMenu(cb_menu);
    glutAddMenuEntry("Orthographic projection", 0);
    glutAddMenuEntry("Perspective projection", 1);;
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    printf("init OK!");
}

void display()
{
    int width = glutGet(GLUT_WINDOW_WIDTH); // get window width
    int height = glutGet(GLUT_WINDOW_HEIGHT); // get window height
    double aspect = 1.0 * width / height;
    glUniformMatrix4fv(2, 1, GL_FALSE, value_ptr(camera.get_viewing())); // to set uniform variable where the location is 2(mat4 V)
    glUniformMatrix4fv(3, 1, GL_FALSE, value_ptr(camera.get_projection(aspect))); // to set uniform variable where the location is 3(mat4 P)

    using namespace glm;
    using namespace std;
    using namespace tinyobj;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear buffer
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // fill the faces by color
    glEnable(GL_POLYGON_OFFSET_FILL); // when the polygons have same z-value, we can set priority
    glPolygonOffset(1, 1); // set offset


    if (show_grid) {
        mat4 grid_mat(1.0f);
        glUniform1i(UVARS("ColorMode"), 2);
        draw_grid(value_ptr(grid_mat));
        glUniform1i(UVARS("ColorMode"), 0);
    }

    if (is_obj_valid) { // cube
        glUseProgram(program[shading_mode]);
        glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(T));
        draw_obj_model(0);
    }

    glDisable(GL_POLYGON_OFFSET_FILL); // disable offset fill

    if (show_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // to draw vertices by line
        glLineWidth(1); // set line width

        // glUniformMatrix4fv(1, 1, GL_FALSE, value_ptr(T));
        draw_obj_model(1);
    }

    glFlush(); // apply the changes immediately
    glutPostRedisplay(); // redraw images.
}

glm::mat4 parallel(double r, double aspect, double n, double f) // to get parallel matrix
{
    double l = -r;
    double width = 2 * r;
    double height = width / aspect;
    double t = height / 2;
    double b = -t;
    return glm::ortho(l, r, b, t, n, f);
}


void mouse(int button, int state, int x, int y)
{
    button_pressed[button] = state;
    mouse_pos[0] = x;
    mouse_pos[1] = y;
}

void motion(int x, int y)
{
    using namespace glm;
    int modifiers = glutGetModifiers();
    int is_alt_active = modifiers & GLUT_ACTIVE_ALT;
    int is_ctrl_active = modifiers & GLUT_ACTIVE_CTRL;
    int is_shift_active = modifiers & GLUT_ACTIVE_SHIFT;
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    GLfloat dx = 1.f * (x - mouse_pos[0]) / w;
    GLfloat dy = -1.f * (y - mouse_pos[1]) / h;

    if (button_pressed[GLUT_LEFT_BUTTON] == GLUT_DOWN)
    {
        if (is_alt_active)
        {
            vec4 disp(camera.eye - camera.center, 1);

            GLfloat alpha = 2.0f;
            mat4 V = camera.get_viewing();
            mat4 Rx = rotate(mat4(), alpha * dy, vec3(transpose(V)[0]));
            mat4 Ry = rotate(mat4(), -alpha * dx, vec3(0, 1, 0));
            mat4 R = Ry * Rx;
            camera.eye = camera.center + vec3(R * disp);
            camera.up = mat3(R) * camera.up;
        }
    }

    if (button_pressed[GLUT_MIDDLE_BUTTON] == GLUT_DOWN)
    {
        if (is_alt_active) {
            mat4 VT = transpose(camera.get_viewing());
            camera.eye += vec3(-dx * VT[0] + -dy * VT[1]);
            camera.center += vec3(-dx * VT[0] + -dy * VT[1]);
        }
    }

    mouse_pos[0] = x;
    mouse_pos[1] = y;
    glutPostRedisplay();
}

void mouse_wheel(int wheel, int dir, int x, int y)
{
    int is_alt_active = glutGetModifiers() & GLUT_ACTIVE_ALT;
    if (is_alt_active) {
        glm::vec3 disp = camera.eye - camera.center;
        if (dir > 0)
            camera.eye = camera.center + 0.95f * disp;
        else
            camera.eye = camera.center + 1.05f * disp;
    }
    else
    {
        if (dir > 0)
            camera.zoom_factor *= 0.95f;
        else
            camera.zoom_factor *= 1.05f;
    }
    glutPostRedisplay();
}
void cb_special(int key, int x, int y) {
    using namespace glm;
    switch (key) {
    case GLUT_KEY_LEFT: T = translate(T, vec3(-0.05f, 0.0f, 0.0f)); TL = rotate(TL, radians(5.0f), vec3(0.0f, +1.0f, 0.0f)); TR = rotate(TR, radians(5.0f), vec3(0.0f, +1.0f, 0.0f)); glutPostRedisplay(); break;
    case GLUT_KEY_RIGHT: T = translate(T, vec3(+0.05f, 0.0f, 0.0f));  TL = rotate(TL, radians(5.0f), vec3(0.0f, -1.0f, 0.0f)); TR = rotate(TR, radians(5.0f), vec3(0.0f, -1.0f, 0.0f)); glutPostRedisplay(); break;
    case GLUT_KEY_UP: T = translate(T, vec3(0.0f, +0.05f, 0.0f)); glutPostRedisplay(); break;
    case GLUT_KEY_DOWN: T = translate(T, vec3(0.0f, -0.05f, 0.0f));  glutPostRedisplay(); break;
    case GLUT_KEY_HOME: T = rotate(T, radians(1.0f), vec3(0.0f, 1.0f, 0.0f)); TL = rotate(TL, radians(5.0f), vec3(0.0f, +1.0f, 0.0f)); TR = rotate(TR, radians(5.0f), vec3(0.0f, -1.0f, 0.0f)); glutPostRedisplay(); break;
    case GLUT_KEY_END: T = rotate(T, radians(1.0f), vec3(0.0f, -1.0f, 0.0f)); TL = rotate(TL, radians(5.0f), vec3(0.0f, -1.0f, 0.0f)); TR = rotate(TR, radians(5.0f), vec3(0.0f, +1.0f, 0.0f)); glutPostRedisplay(); break;
    }
}

void cb_menu(int value)
{
    if (value == 0) {
        camera.projection_mode = 0; // orthgonal
        glutPostRedisplay();
    }
    else {
        camera.projection_mode = 1; // perspective
        glutPostRedisplay();
    }
}


void draw_grid(const GLfloat* trans_mat) // to draw cube vertices
{
    glUniformMatrix4fv(1, 1, GL_FALSE, trans_mat); // to set uniform variable where the location is 1 (mat4 M)
    glBindVertexArray(grid_vao); // bind vao[0]
    glDrawArrays(GL_LINES, 0, grid_vtx_pos.size() / 3); // draw cube
}


void make_grid(GLfloat w, GLfloat h, int m, int n) {
    get_grid(grid_vtx_pos, w, h, m, n); // get cube vertice positions
    glGenVertexArrays(1, &grid_vao); // generate VAO
    glBindVertexArray(grid_vao); // bind vao
    glGenBuffers(1, &grid_vbo);  // generate VBOs
    bind_buffer(grid_vbo, grid_vtx_pos, program[shading_mode], "vPosition", 3); // assign vertice position data to "vPosition" variable which is in the program
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '1': shading_mode = 0; glutPostRedisplay(); break; // phong
    case '2': shading_mode = 1; glutPostRedisplay(); break; // gouraud
    case 'w': show_wireframe = !show_wireframe; glutPostRedisplay(); break; // redraw display.
    case 'o': camera.projection_mode = 0; glutPostRedisplay(); break; // redraw display.
    case 'p': camera.projection_mode = 1; glutPostRedisplay(); break; // redraw display.
    case 'g': show_grid = !show_grid; glutPostRedisplay(); break;
    }
}

void bind_buffer(GLint buffer, GLvec& vec, int program, const GLchar* attri_name, GLint attri_size)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer); // bind buffer with GL_ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vec.size(), vec.data(), GL_STATIC_DRAW); // allocate data to GL_ARRAY_BUFFER.
    GLint location = glGetAttribLocation(program, attri_name); // get location of attri_name in compiled program
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0); // specifies how to read the buffer data through the attribute
    glEnableVertexAttribArray(location);  // enable vertex attribute array
}

void bind_buffer(GLint buffer, int program, const GLchar* attri_name, GLint attri_size)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    GLuint location = glGetAttribLocation(program, attri_name);
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
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

// to push vx, vy, vz data to vector p
#define FPUSH_VTX3_AT(p, i, vx,vy,vz)\
    do{\
        size_t i3 = 3*(i);\
        p[i3+0] = (float)(vx);\
        p[i3+1] = (float)(vy);\
        p[i3+2] = (float)(vz);\
    } while (0)


void get_grid(std::vector<GLfloat>& p, GLfloat w, GLfloat h, int m, int n)
{
    GLfloat x0 = -0.5f * w;
    GLfloat x1 = +0.5f * w;
    GLfloat z0 = -0.5f * h;
    GLfloat z1 = +0.5f * h;
    for (int i = 0; i <= m; ++i) {
        GLfloat x = x0 + w * i / m;
        FPUSH_VTX3(p, x, 0, z0);
        FPUSH_VTX3(p, x, 0, z1);
    }
    for (int i = 0; i <= n; ++i) {
        GLfloat z = z0 + h * i / n;
        FPUSH_VTX3(p, x0, 0, z);
        FPUSH_VTX3(p, x1, 0, z);
    }
}