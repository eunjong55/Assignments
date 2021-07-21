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

typedef std::vector<GLfloat> GLvec;

void init(); // to get vertex position and vertex color
void display(); // draw vertices 
void bind_buffer(GLint buffer, GLvec& vec, int program, const GLchar* attri_name, GLint attri_size); // assign data to attri_name
int build_program(); // build shader

glm::mat4 parallel(double r, double aspect, double n, double f);

void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void mouse_wheel(int wheel, int dir, int x, int y);
void cb_special(int key, int x, int y);
void cb_menu(int value);
void keyboard(unsigned char key, int x, int y); // keyboard event function

void get_grid(std::vector<GLfloat>& p, GLfloat w, GLfloat h, int m, int n);
void get_color_3d_by_pos(GLvec& c, GLvec& p, GLfloat offset); // to get vertex color by vertex position
void get_box_3d(std::vector<GLfloat>& p, GLfloat lx, GLfloat ly, GLfloat lz); // to get cube vertices position
void get_sphere_3d(GLvec& p, GLfloat r, GLint subh, GLint suba); // to get sphere vertices position
void get_cone_3d(std::vector<GLfloat>& p, std::vector<size_t>& side_idx, std::vector<size_t>& bottom_idx, GLfloat radius, GLfloat height, GLint n); // to get cone vertices position
void get_cylinder_3d(std::vector<GLfloat>& p, std::vector<size_t>& side_idx, std::vector<size_t>& top_idx, std::vector<size_t>& bottom_idx, GLfloat radius, GLfloat height, GLint n); // to get cyliner vertices position
void get_torus_3d(std::vector<GLfloat>& p, std::vector<std::vector<size_t>>& side_idx, GLfloat r0, GLfloat r1, GLint na, GLint nh); // to get torus vertices position

void make_grid(GLfloat w, GLfloat h, int m, int n);
void make_cube(GLfloat radius, GLfloat height, GLuint subdivs); // to get cube vertices position and register vertices information to VAO and VBO
void make_sphere(GLfloat radius, GLfloat height, GLuint subdivs); // to get sphere vertices position and register vertices information to VAO and VBO
void make_cone(GLfloat radius, GLfloat height, GLuint subdivs); // to get cone vertices position and register vertices information to VAO and VBO
void make_cylinder(GLfloat radius, GLfloat height, GLuint subdivs); // to get cyliner vertices position and register vertices information to VAO and VBO
void make_torus(GLfloat r0, GLfloat r1, GLuint na, GLuint nh); // to get torus vertices position and register vertices information to VAO and VBO

void draw_grid(const GLfloat* trans_mat); // to draw cube vertices
void draw_cube(const GLfloat* trans_mat); // to draw cube vertices
void draw_sphere(const GLfloat* trans_mat);
void draw_cone(const GLfloat* trans_mat); // to draw cone vertices
void draw_cylinder(const GLfloat* trans_mat);
void draw_torus(const GLfloat* trans_mat);
void draw_composite_model(GLfloat theta);


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

GLuint program;
Camera camera;

bool show_wireframe = FALSE;
bool show_grid = FALSE;
bool show_axis = FALSE;

glm::mat4 T(1.0f); // for object rotation
glm::mat4 TL(1.0f); // for left wheel rotation
glm::mat4 TR(1.0f); // for right wheel rotation

GLuint vao[7], buffs[7][2]; // VAOs and VBOs
GLvec vtx_pos[7], vtx_clrs[7]; // to store vertex postion and vertex color
GLuint cone_element_buffs[2]; // to use drawElements methods
GLuint cylinder_element_buffs[3]; // to use drawElements methods
GLuint torus_element_buffs[100]; // to use drawElements methods

std::vector<size_t> cone_idx_list[2];
std::vector<size_t> cylinder_idx_list[3];
std::vector<std::vector<size_t>> torus_idx_list;

GLuint active_vao = 0;
int button_pressed[3] = { GLUT_UP, GLUT_UP, GLUT_UP };
int mouse_pos[2] = { 0, 0 };

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
    program = build_program(); // build a program

    make_grid(3, 3, 10, 10);
    make_cube(1, 1, 1); //  get cube vertices position and register vertices information to VAO and VBO
    make_sphere(0.5, 20, 20); //  get sphere vertices position and register vertices information to VAO and VBO
    make_cone(0.5, 1, 10); //  get cone vertices position and register vertices information to VAO and VBO
    make_cylinder(0.5, 1, 20); //  get cylinder vertices position and register vertices information to VAO and VBO
    make_torus(0.3, 0.3, 10, 20); //  get torus vertices position and register vertices information to VAO and VBO
    glEnable(GL_DEPTH_TEST); // for depth testing
    glDepthFunc(GL_LESS); // set depth test function

    int menu_id = glutCreateMenu(cb_menu);
    glutAddMenuEntry("Orthographic projection", 0);
    glutAddMenuEntry("Perspective projection", 1);;
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void display()
{
    GLint location = glGetUniformLocation(program, "M");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear buffer
    GLfloat theta = 0.001f * clock(); // theta is linearly increasd

    using namespace glm;

    int width = glutGet(GLUT_WINDOW_WIDTH); // get window width
    int height = glutGet(GLUT_WINDOW_HEIGHT); // get window height
    double aspect = 1.0 * width / height;
    glUniformMatrix4fv(2, 1, GL_FALSE, value_ptr(camera.get_viewing())); // to set uniform variable where the location is 2(mat4 V)
    glUniformMatrix4fv(3, 1, GL_FALSE, value_ptr(camera.get_projection(aspect))); // to set uniform variable where the location is 3(mat4 P)

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // fill the faces by color
    glEnable(GL_POLYGON_OFFSET_FILL); // when the polygons have same z-value, we can set priority
    glPolygonOffset(1, 1); // set offset

    glUniform1i(4, 0); // to set uniform variable where the location is 4

    if (show_grid) {
        mat4 grid_mat(1.0f);
        glUniform1i(4, 5);
        draw_grid(value_ptr(grid_mat));
        glUniform1i(4, 0);
    }
    if (show_axis) {

        mat4 x_axis_mat(1.0f);
        x_axis_mat = translate(x_axis_mat, vec3(0.5f, 0.0f, 0.0f));
        x_axis_mat = rotate(x_axis_mat, radians(90.0f), vec3(0.0f, 0.0f, 0.1f)); // rotate the cube around the y-axis
        x_axis_mat = scale(x_axis_mat, vec3(0.05f, 1.0f, 0.05f)); // scaling

        mat4 y_axis_mat(1.0f);
        y_axis_mat = translate(y_axis_mat, vec3(0.0f, 0.5f, 0.0f));
        y_axis_mat = rotate(y_axis_mat, radians(90.0f), vec3(0.0f, 1.0f, 0.0f)); // rotate the cube around the y-axis
        y_axis_mat = scale(y_axis_mat, vec3(0.05f, 1.0f, 0.05f)); // scaling 

        mat4 z_axis_mat(1.0f);
        z_axis_mat = translate(z_axis_mat, vec3(0.0f, 0.0f, 0.5f));
        z_axis_mat = rotate(z_axis_mat, radians(90.0f), vec3(1.0f, 0.0f, 0.0f)); // rotate the cube around the y-axis
        z_axis_mat = scale(z_axis_mat, vec3(0.05f, 1.0f, 0.05f)); // scaling

        glUniform1i(4, 2);
        draw_cylinder(value_ptr(T * x_axis_mat));
        glUniform1i(4, 3);
        draw_cylinder(value_ptr(T * y_axis_mat));
        glUniform1i(4, 4);
        draw_cylinder(value_ptr(T * z_axis_mat));
        glUniform1i(4, 0);
    }

    if (active_vao == 0) { // cube
        draw_cube(value_ptr(T));
    }
    else if (active_vao == 1) { // sphere
        draw_sphere(value_ptr(T));
    }
    else if (active_vao == 2) { // cone
        draw_cone(value_ptr(T));
    }
    else if (active_vao == 3) { // cylinder
        draw_cylinder(value_ptr(T));
    }
    else if (active_vao == 4) { // torus
        draw_torus(value_ptr(T));
    }
    else if (active_vao == 5) { // composite model
        draw_composite_model(theta);
    }
    glDisable(GL_POLYGON_OFFSET_FILL); // disable offset fill

    if (show_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // to draw vertices by line
        glLineWidth(1); // set line width
        glUniform1i(4, 1); // to set uniform variable where the location is 4
        if (active_vao == 0) { // cube
            draw_cube(value_ptr(T));
        }
        else if (active_vao == 1) { // sphere
            draw_sphere(value_ptr(T));
        }
        else if (active_vao == 2) { // cone
            draw_cone(value_ptr(T));
        }
        else if (active_vao == 3) { // cylinder
            draw_cylinder(value_ptr(T));
        }
        else if (active_vao == 4) { // torus
            draw_torus(value_ptr(T));
        }
        else if (active_vao == 5) {
            draw_composite_model(theta);
        }
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
    glBindVertexArray(vao[6]); // bind vao[0]
    glDrawArrays(GL_LINES, 0, vtx_pos[6].size() / 3); // draw cube
}


void draw_cube(const GLfloat* trans_mat) // to draw cube vertices
{
    glUniformMatrix4fv(1, 1, GL_FALSE, trans_mat); // to set uniform variable where the location is 1 (mat4 M)
    glBindVertexArray(vao[0]); // bind vao[0]
    glDrawArrays(GL_TRIANGLES, 0, vtx_pos[0].size() / 3); // draw cube
}

void draw_sphere(const GLfloat* trans_mat)
{
    glUniformMatrix4fv(1, 1, GL_FALSE, trans_mat); // to set uniform variable where the location is 1 (mat4 M)
    glBindVertexArray(vao[1]); // bind vao[1]
    glDrawArrays(GL_TRIANGLES, 0, vtx_pos[1].size() / 2); // draw sphere
}

void draw_cone(const GLfloat* trans_mat) // to draw cone vertices
{
    glUniformMatrix4fv(1, 1, GL_FALSE, trans_mat); // to set uniform variable where the location is 1 (mat4 M)
    glBindVertexArray(vao[2]); // bind vao[2]
    for (int i = 0; i < 2; ++i)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cone_element_buffs[i]); // bind element array buffer with cone_element_buffs
        glDrawElements(GL_TRIANGLE_FAN, cone_idx_list[i].size(), GL_UNSIGNED_INT, NULL); // draw cone
    }
}
void draw_cylinder(const GLfloat* trans_mat)
{
    glUniformMatrix4fv(1, 1, GL_FALSE, trans_mat); // to set uniform variable where the location is 1 (mat4 M)
    std::vector<size_t>* idx_list[] = { &cylinder_idx_list[0], &cylinder_idx_list[1], &cylinder_idx_list[2] };
    GLuint drawing_mode[3] = { GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLE_FAN }; // set drawing mode

    glBindVertexArray(vao[3]); // bind vao[3]
    for (int i = 0; i < 3; ++i) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinder_element_buffs[i]); // bind element array buffer with cylinder_element_buffs
        glDrawElements(drawing_mode[i], idx_list[i]->size(), GL_UNSIGNED_INT, NULL); // draw cylinder
    }
}
void draw_torus(const GLfloat* trans_mat)
{
    glUniformMatrix4fv(1, 1, GL_FALSE, trans_mat); // to set uniform variable where the location is 1 (mat4 M)
    glBindVertexArray(vao[4]); // bind vao[4]
    size_t n = torus_idx_list.size();
    for (int i = 0; i < n; ++i)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torus_element_buffs[i]); // bind element array buffer with torus_element_buffs
        glDrawElements(GL_TRIANGLE_STRIP, torus_idx_list[i].size(), GL_UNSIGNED_INT, NULL); // draw torus
    }
}
void draw_composite_model(GLfloat theta) {
    using namespace glm;

    mat4 cube_mat1(1.0f);
    cube_mat1 = translate(cube_mat1, vec3(-0.3f, 0.3f, 0.0f)); // translate the cube
    cube_mat1 = scale(cube_mat1, vec3(0.6f)); // scaling
    draw_cube(value_ptr(T*cube_mat1)); // draw cube

    mat4 cube_mat2(1.0f);
    cube_mat2 = translate(cube_mat2, vec3(0.0f, -0.2f, 0.0f)); // translate the cube
    cube_mat2 = scale(cube_mat2, vec3(1.2, 0.4, 0.6)); // scaling
    draw_cube(value_ptr(T*cube_mat2)); // draw cube

    mat4 cone_mat(1.0f);
    cone_mat = translate(cone_mat, vec3(-0.3f, +0.7f, 0.0f)); // rotate the cone around the y-axis
    cone_mat = scale(cone_mat, vec3(1.0, 0.2, 1.0)); // scaling
    draw_cone(value_ptr(T*cone_mat));

    mat4 torus_mat1(1.0f);

    torus_mat1 = translate(torus_mat1, vec3(0.3f, -0.4f, -0.5f));
    torus_mat1 = rotate(torus_mat1, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    torus_mat1 = scale(torus_mat1, vec3(0.3)); // scaling
    draw_torus(value_ptr(T*torus_mat1*TL));

    mat4 torus_mat2(1.0f);
    torus_mat2 = translate(torus_mat2, vec3(0.3f, -0.4f, +0.5f));
    torus_mat2 = rotate(torus_mat2, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    torus_mat2 = scale(torus_mat2, vec3(0.3)); // scaling
    draw_torus(value_ptr(T*torus_mat2*TR));

    mat4 torus_mat3(1.0f);
    torus_mat3 = translate(torus_mat3, vec3(-0.3f, -0.4f, +0.5f));
    torus_mat3 = rotate(torus_mat3, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    torus_mat3 = scale(torus_mat3, vec3(0.3)); // scaling
    draw_torus(value_ptr(T*torus_mat3*TR));

    mat4 torus_mat4(1.0f);
    torus_mat4 = translate(torus_mat4, vec3(-0.3f, -0.4f, -0.5f));
    torus_mat4 = rotate(torus_mat4, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    torus_mat4 = scale(torus_mat4, vec3(0.3)); // scaling
    draw_torus(value_ptr(T*torus_mat4*TL));

    mat4 cylinder_mat1(1.0f);
    cylinder_mat1 = translate(cylinder_mat1, vec3(+0.3f, -0.4f, 0.0f));
    cylinder_mat1 = rotate(cylinder_mat1, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    cylinder_mat1 = scale(cylinder_mat1, vec3(0.1, 1.0, 0.1));
    draw_cylinder(value_ptr(T*cylinder_mat1));

    mat4 cylinder_mat2(1.0f);
    cylinder_mat2 = translate(cylinder_mat2, vec3(-0.3f, -0.4f, 0.0f));
    cylinder_mat2 = rotate(cylinder_mat2, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    cylinder_mat2 = scale(cylinder_mat2, vec3(0.1, 1.0, 0.1));
    draw_cylinder(value_ptr(T*cylinder_mat2));

    mat4 cylinder_mat3(1.0f);
    
    cylinder_mat3 = translate(cylinder_mat3, vec3(0.25f, 0.0f, 0.0f));
    cylinder_mat3 = rotate(cylinder_mat3, radians(90.0f), vec3(0.0f, 0.0f, 1.f));
    cylinder_mat3 = scale(cylinder_mat3, vec3(0.5));
    draw_cylinder(value_ptr(T*cylinder_mat3*TL));

    mat4 sphere_mat1(1.0f);
    sphere_mat1 = translate(sphere_mat1, vec3(0.6f, -0.2f, 0.1f));
    sphere_mat1 = scale(sphere_mat1, vec3(0.1));
    draw_sphere(value_ptr(T*sphere_mat1));

    mat4 sphere_mat2(1.0f);
    sphere_mat2 = translate(sphere_mat2, vec3(0.6f, -0.2f, -0.1f));
    sphere_mat2 = scale(sphere_mat2, vec3(0.1));
    draw_sphere(value_ptr(T*sphere_mat2));


}

void make_grid(GLfloat w, GLfloat h, int m, int n) {
    get_grid(vtx_pos[6], w, h, m, n); // get cube vertice positions
    get_color_3d_by_pos(vtx_clrs[6], vtx_pos[6], 0); // get colors
    glGenVertexArrays(1, &vao[6]); // generate VAO
    glBindVertexArray(vao[6]); // bind vao
    glGenBuffers(2, buffs[6]);  // generate VBOs
    bind_buffer(buffs[6][0], vtx_pos[6], program, "vPosition", 3); // assign vertice position data to "vPosition" variable which is in the program
    bind_buffer(buffs[6][1], vtx_clrs[6], program, "vColor", 3); // assign vertice position data to "vColor" variable which is in the program
}

void make_cube(GLfloat radius, GLfloat height, GLuint subdivs) {
    get_box_3d(vtx_pos[0], 1, 1, 1); // get cube vertice positions
    get_color_3d_by_pos(vtx_clrs[0], vtx_pos[0], 0); // get colors
    glGenVertexArrays(1, &vao[0]); // generate VAO
    glBindVertexArray(vao[0]); // bind vao
    glGenBuffers(2, buffs[0]);  // generate VBOs
    bind_buffer(buffs[0][0], vtx_pos[0], program, "vPosition", 3); // assign vertice position data to "vPosition" variable which is in the program
    bind_buffer(buffs[0][1], vtx_clrs[0], program, "vColor", 3); // assign vertice position data to "vColor" variable which is in the program
}
void make_sphere(GLfloat radius, GLfloat height, GLuint subdivs) {
    get_sphere_3d(vtx_pos[1], radius, height, subdivs); // get sphere vertice positions
    get_color_3d_by_pos(vtx_clrs[1], vtx_pos[1], 0); // get colors
    glGenVertexArrays(1, &vao[1]); // generate VAO
    glBindVertexArray(vao[1]); // bind vao
    glGenBuffers(2, buffs[1]);  // generate VBOs
    bind_buffer(buffs[1][0], vtx_pos[1], program, "vPosition", 3); // assign vertice position data to "vPosition" variable which is in the program
    bind_buffer(buffs[1][1], vtx_clrs[1], program, "vColor", 3);  // assign vertice color data to "vColor" variable which is in the program
}
void make_cone(GLfloat radius, GLfloat height, GLuint subdivs) {
    get_cone_3d(vtx_pos[2], cone_idx_list[0], cone_idx_list[1], radius, height, subdivs); // get cone vertice positions
    get_color_3d_by_pos(vtx_clrs[2], vtx_pos[2], 0); // get colors
    glGenVertexArrays(1, &vao[2]); // generate VAO
    glBindVertexArray(vao[2]); // bind vao
    const char* cone_attri_name[2] = { "vPosition", "vColor" };
    GLvec* cone_vtx_list[2] = { &vtx_pos[2], &vtx_clrs[2] };
    glGenBuffers(2, buffs[2]);  // generate VBOs
    for (int i = 0; i < 2; ++i) {
        bind_buffer(buffs[2][i], *cone_vtx_list[i], program, cone_attri_name[i], 3); // assign vertice position data to attri_name variable which is in the program
    }
    glGenBuffers(2, cone_element_buffs);  // generate VBOs
    for (int i = 0; i < 2; ++i)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cone_element_buffs[i]);  // bind element array buffer with cone_element_buffs
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(size_t) * cone_idx_list[i].size(),
            cone_idx_list[i].data(), GL_STATIC_DRAW);  // assign cone vertex index data to element array buffer
    }
}
void make_cylinder(GLfloat radius, GLfloat height, GLuint subdivs) 
{
    get_cylinder_3d(vtx_pos[3], cylinder_idx_list[0], cylinder_idx_list[1], cylinder_idx_list[2], radius, height, subdivs);  // get cylinder vertice positions
    get_color_3d_by_pos(vtx_clrs[3], vtx_pos[3], 0); // get colors
    glGenVertexArrays(1, &vao[3]); // generate VAO
    glBindVertexArray(vao[3]); // bind vao
    const char* cylinder_attri_name[2] = { "vPosition", "vColor" };
    GLvec* cylinder_vtx_list[2] = { &vtx_pos[3], &vtx_clrs[3] };
    glGenBuffers(2, buffs[3]);  // generate VBOs
    for (int i = 0; i < 2; ++i) {
        bind_buffer(buffs[3][i], *cylinder_vtx_list[i], program, cylinder_attri_name[i], 3); // assign vertice position data to attri_name variable which is in the program
    }
    glGenBuffers(3, cylinder_element_buffs);  // generate VBOs
    for (int i = 0; i < 3; ++i)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinder_element_buffs[i]);  // bind element array buffer with cylinder_element_buffs
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(size_t) * cylinder_idx_list[i].size(),
            cylinder_idx_list[i].data(), GL_STATIC_DRAW); // assign cylinder vertex index data to element array buffer
    }
}

void make_torus(GLfloat r0, GLfloat r1, GLuint na, GLuint nh) 
{ 
    get_torus_3d(vtx_pos[4], torus_idx_list, r0, r1, na, nh);  // get torus vertice positions
    get_color_3d_by_pos(vtx_clrs[4], vtx_pos[4], 0); // get colors
    glGenVertexArrays(1, &vao[4]); // generate VAO
    glBindVertexArray(vao[4]); // bind vao
    const char* torus_attri_name[2] = { "vPosition", "vColor" };
    GLvec* torus_vtx_list[2] = { &vtx_pos[4], &vtx_clrs[4] };
    glGenBuffers(2, buffs[4]);  // generate VBOs
    for (int i = 0; i < 2; ++i) {
        bind_buffer(buffs[4][i], *torus_vtx_list[i], program, torus_attri_name[i], 3); // assign vertice position data to attri_name variable which is in the program
    }
    glGenBuffers(na, torus_element_buffs);  // generate VBOs
    for (int i = 0; i < na; ++i)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torus_element_buffs[i]);  // bind element array buffer with torus_element_buffs
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(size_t) * torus_idx_list[i].size(),
            torus_idx_list[i].data(), GL_STATIC_DRAW);  // assign torus vertex index data to element array buffer
    }
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case '1': active_vao = 0; glutPostRedisplay(); break; // redraw display.
        case '2': active_vao = 1; glutPostRedisplay(); break; // redraw display.
        case '3': active_vao = 2; glutPostRedisplay(); break; // redraw display.
        case '4': active_vao = 3; glutPostRedisplay(); break; // redraw display.
        case '5': active_vao = 4; glutPostRedisplay(); break; // redraw display.
        case '6': active_vao = 5; glutPostRedisplay(); break; // redraw display.
        case 'w': show_wireframe = !show_wireframe; glutPostRedisplay(); break; // redraw display.
        case 'o': camera.projection_mode = 0; glutPostRedisplay(); break; // redraw display.
        case 'p': camera.projection_mode = 1; glutPostRedisplay(); break; // redraw display.
        case 'g': show_grid = !show_grid; glutPostRedisplay(); break;
        case 'a': show_axis = !show_axis; glutPostRedisplay(); break;
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

int build_program()
{
    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER, "hw9.vert" },
        { GL_FRAGMENT_SHADER, "hw9.frag" },
        { GL_NONE, NULL }
    };
    GLuint program = LoadShaders(shaders); // compile shaders and link program with compiled shaders 
    glUseProgram(program); // to use program
    return program;
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

void get_box_3d(std::vector<GLfloat>& p, GLfloat lx, GLfloat ly, GLfloat lz) // get cube vertices positions
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

void get_sphere_3d(GLvec& p, GLfloat r, GLint subh, GLint suba) //to get sphere vertices position
{
    for (int i = 1; i <= subh; ++i)
    {
        double theta0 = M_PI * (i - 1) / subh; // get angle of i-1
        double theta1 = M_PI * i / subh;

        double y0 = r * cos(theta0);
        double rst0 = r * sin(theta0);
        double y1 = r * cos(theta1);
        double rst1 = r * sin(theta1);

        for (int j = 1; j <= suba; ++j)
        {
            double phi0 = 2 * M_PI * (j - 1) / suba;
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

void get_cone_3d(std::vector<GLfloat>& p, std::vector<size_t>& side_idx, std::vector<size_t>& bottom_idx, GLfloat radius, GLfloat height, GLint n) //to get cone vertices position
{
    GLfloat half_height = height / 2;
    GLfloat theta, x, z;

    FPUSH_VTX3(p, 0, half_height, 0);
    side_idx.push_back(0);
    for (int i = 0; i <= n; ++i)
    {
        theta = (GLfloat)(2.0 * M_PI * i / n);
        x = radius * sin(theta);
        z = radius * cos(theta);
        FPUSH_VTX3(p, x, -half_height, z);
        side_idx.push_back(i + 1);
        bottom_idx.push_back(n + 2 - i);
    }
    FPUSH_VTX3(p, 0, -half_height, 0);
    bottom_idx.push_back(1);
}

void get_cylinder_3d(std::vector<GLfloat>& p, std::vector<size_t>& side_idx, std::vector<size_t>& top_idx, std::vector<size_t>& bottom_idx, GLfloat radius, GLfloat height, GLint n) //to get cylinder vertices position
{
    GLfloat half_height = height / 2;
    GLfloat theta, x, z;
    p.resize(3 * (2 * n + 4));

    FPUSH_VTX3_AT(p, 0, 0, half_height, 0);
    top_idx.push_back(0);
    bottom_idx.push_back(2 * n + 3);
    for (int i = 0; i <= n; ++i) {
        theta = (GLfloat)(2.0 * M_PI * i / n);
        x = radius * sin(theta);
        z = radius * cos(theta);
        FPUSH_VTX3_AT(p, 2 * i + 1, x, half_height, z);
        FPUSH_VTX3_AT(p, 2 * i + 2, x, -half_height, z);
        side_idx.push_back(2 * i + 1);
        side_idx.push_back(2 * i + 2);
        top_idx.push_back(2 * i + 1);
        bottom_idx.push_back(2 * n + 2 - 2 * i);
    }
    FPUSH_VTX3_AT(p, 2 * n + 3, 0, -half_height, 0);
}

void get_torus_3d(std::vector<GLfloat>& p, std::vector<std::vector<size_t>>& side_idx, GLfloat r0, GLfloat r1, GLint na, GLint nh) //to get torus vertices position
{
    GLfloat theta0, theta1;
    GLfloat x0, y0, z0;
    y0 = 0;
    GLfloat x, y, z;
    p.resize(10*(na + 1) * (nh + 1));
    int k = 0;
    for (int i = 0; i <= na; i++) {
        theta0 = (GLfloat)(2.0 * M_PI * i / na);
        x0 = (r0 + r1) * sin(theta0);
        z0 = (r0 + r1) * cos(theta0);

        for (int j = 0; j <= nh; j++) {
            theta1 = (GLfloat)(2.0 * M_PI * j / nh);
            x = x0 + r1 * cos(theta1) * sin(theta0);
            y = r1 * sin(theta1);
            z = z0 + r1 * cos(theta1) * cos(theta0);
            FPUSH_VTX3_AT(p, k, x, y, z);
            k++;
        }
    }
    for (int i = 0; i < na; i++) {
        std::vector<size_t> tmp;
        for (int j = 0; j <= nh; j++) {
            tmp.push_back(i * (nh+1) + j);
            tmp.push_back((i + 1) * (nh + 1) + j);
        }
        side_idx.push_back(tmp);
    }
}