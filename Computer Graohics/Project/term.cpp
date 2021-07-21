#include <stdio.h>
#include <stdlib.h>
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
#include <iostream>

typedef std::vector<GLfloat> GLvec;

void init(); // to get vertex position and vertex color
void display(); // draw vertices 
void bind_buffer(GLint buffer, GLvec& vec, int program, const GLchar* attri_name, GLint attri_size); // assign data to attri_name
int build_program(); // build shader

glm::mat4 parallel(double r, double aspect, double n, double f);

void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void mouse_wheel(int wheel, int dir, int x, int y);
void specialkey(int key, int x, int y);
void cb_menu(int value);
void keyboard(unsigned char key, int x, int y); // keyboard event function

void get_grid(std::vector<GLfloat>& p, GLfloat w, GLfloat h, int m, int n);
void get_color_3d_by_pos(GLvec& c, GLvec& p, GLfloat offset); // to get vertex color by vertex position
void get_box_3d(std::vector<GLfloat>& p, GLfloat lx, GLfloat ly, GLfloat lz); // to get cube vertices position

void make_grid(GLfloat w, GLfloat h, int m, int n);
void make_cube(GLfloat length); // to get cube vertices position and register vertices information to VAO and VBO

void draw_grid(const GLfloat* trans_mat); // to draw cube vertices
void draw_cube(const GLfloat* trans_mat); // to draw cube vertices



const int map_width = 5; //map width
const int map_height = 10; //map height

bool map[map_width][map_height + 5][map_width] = { 0, }; // to store stacked blocks

int score = 0; //game score

void DrawMap() // to draw stackted blocks.
{
    for (int i = 0; i < map_width; i++)
    {
        for (int j = 0; j < map_height; j++)
        {
            for (int k = 0; k < map_width; k++)
            {
                if (map[i][j][k])
                {
                    using namespace glm;
                    mat4 trans_mat(1.0f);
                    trans_mat = translate(trans_mat, vec3(i - (map_width / 2), j - (map_height / 2), k - (map_width / 2))); //+(map_width / 2)
                    draw_cube(value_ptr(trans_mat));
                }
            }
        }
    }
}
bool IsGameEnd() // to check game end
{
    for (int i = 0; i < map_width; i++)
    {
        for (int j = 0; j < map_width; j++)
        {
            if (map[i][9][j] == true) return true;
        }
    }
    return false;
}

void MapInit() // to initialize the map
{
    for (int i = 0; i < map_width; i++)
    {
        for (int j = 0; j < map_height + 5; j++)
        {
            for (int k = 0; k < map_width; k++)
            {
                map[i][j][k] = 0;
            }
        }
    }
}

void ClearFloor(int idx) // when a floor full,  clear the floor in the map
{
    for (int i = idx; i < map_height - 1; i++)
    {
        for (int j = 0; j < map_width; j++)
        {
            for (int k = 0; k < map_width; k++)
            {
                map[j][i][k] = map[j][i + 1][k];
            }
        }
    }
}

void FillCheck() // check full
{
    for (int i = 0; i < map_height; i++)
    {
        int cnt = 0;
        for (int j = 0; j < map_width; j++)
        {
            for (int k = 0; k < map_width; k++)
            {
                if (map[j][i][k] == 1) cnt += 1;
            }
        }
        if (cnt == map_width * map_width)
        {
            score += 1;
            ClearFloor(i);
        }
    }
}

#define FPUSH_VTX3(p, vx, vy, vz) \
do { \
    p.push_back(vx); \
    p.push_back(vy); \
    p.push_back(vz); \
} while(0)

#define FPUSH_VTX3_AT(p, i, vx,vy,vz)\
    do{\
        size_t i3 = 3*(i);\
        p[i3+0] = vx;\
        p[i3+1] = vy;\
        p[i3+2] = vz;\
    } while (0)

class Block // to manage block efficiently
{
private:
    std::vector<int> location; //Location of blocks.
    int t_x, t_y, t_z; // the translation of blocks.

public:
    Block()
    {
        location.resize(12);
        t_x = 0;
        t_y = 0;
        t_z = 0;
    }

    void init() //Create random blocks from start location.
    {
        t_x = 0;
        t_z = 0;
        t_y = 5;
        srand(time(0));
        int case_ = rand() % 4;
        if (case_ == 0)
        {
            FPUSH_VTX3_AT(location, 0, 0, 0, 0);
            FPUSH_VTX3_AT(location, 1, 0, 1, 0);
            FPUSH_VTX3_AT(location, 2, 0, 2, 0);
            FPUSH_VTX3_AT(location, 3, 1, 0, 0);
        }
        else if (case_ == 1)
        {
            FPUSH_VTX3_AT(location, 0, 0, 0, 0);
            FPUSH_VTX3_AT(location, 1, 0, -1, 0);
            FPUSH_VTX3_AT(location, 2, 0, 1, 0);
            FPUSH_VTX3_AT(location, 3, 0, 2, 0);
        }
        else if (case_ == 2)
        {
            FPUSH_VTX3_AT(location, 0, 0, 0, 0);
            FPUSH_VTX3_AT(location, 1, 0, 1, 0);
            FPUSH_VTX3_AT(location, 2, 1, 0, 0);
            FPUSH_VTX3_AT(location, 3, 1, 1, 0);
        }
        else if (case_ == 3)
        {
            FPUSH_VTX3_AT(location, 0, 0, 0, 0);
            FPUSH_VTX3_AT(location, 1, 0, 1, 0);
            FPUSH_VTX3_AT(location, 2, 1, 0, 0);
            FPUSH_VTX3_AT(location, 3, 1, -1, 0);
        }
    }

    void DrawBlock() //Draw the current location of the block as a cube.
    {
        using namespace glm;
        for (int i = 0; i < 4; i++)
        {
            mat4 trans_mat(1.0f);
            trans_mat = translate(trans_mat, vec3(location[3 * i + 0] + t_x, location[3 * i + 1] + t_y, location[3 * i + 2] + t_z));
            draw_cube(value_ptr(trans_mat));
        }
    }

    void Move(int x, int y, int z) //Move block after feasibility check.
    {
        if (IsFeasible(location, t_x + x, t_y + y, t_z + z)) {
            t_x += x;
            t_y += y;
            t_z += z;
        }
    }

    void Rotation(int x, int y, int z) //Rotation of blocks after feasibility check.
    {
        std::vector<int> temp_loc;
        temp_loc.resize(12);
        for (int i = 0; i < 4; i++)
        {
            float sin_ = 1;
            float cos_ = 0;
            float x_, y_, z_;
            if (x > 0)
            {
                x_ = location[3 * i + 0];
                y_ = location[3 * i + 1] * cos_ - location[3 * i + 2] * sin_;
                z_ = location[3 * i + 1] * sin_ + location[3 * i + 2] * cos_;
            }
            else if (y > 0)
            {
                x_ = location[3 * i + 0] * cos_ + location[3 * i + 2] * sin_;
                y_ = location[3 * i + 1];
                z_ = -location[3 * i + 0] * sin_ + location[3 * i + 2] * cos_;
            }
            else if (z > 0)
            {
                x_ = location[3 * i + 0] * cos_ - location[3 * i + 1] * sin_;
                y_ = location[3 * i + 0] * sin_ + location[3 * i + 1] * cos_;
                z_ = location[3 * i + 2];

            }
            FPUSH_VTX3_AT(temp_loc, i, x_, y_, z_);
        }
        if (IsFeasible(temp_loc, t_x, t_y, t_z)) std::copy(temp_loc.begin(), temp_loc.end(), location.begin());
    }

    bool IsFeasible(std::vector<int> block_loc, int trans_x, int trans_y, int trans_z) //Make sure block is not out of the map or the same location already 
    {

        for (int i = 0; i < 4; i++)
        {
            if ((block_loc[3 * i + 0] + trans_x < -(map_width / 2)) || (block_loc[3 * i + 0] + trans_x > map_width / 2)) return false;
            if ((block_loc[3 * i + 1] + trans_y < -(map_height / 2))) return false;
            if ((block_loc[3 * i + 2] + trans_z < -(map_width / 2)) || (block_loc[3 * i + 2] + trans_z > map_width / 2)) return false;

            if (map[block_loc[3 * i + 0] + trans_x + (map_width / 2)][block_loc[3 * i + 1] + trans_y + (map_height / 2)][block_loc[3 * i + 2] + trans_z + (map_width / 2)] == true) return false;
        }
        return true;
    }


    int IsCollision() //Check for more movement to the y-axis.
    {
        for (int i = 0; i < 4; i++)
        {
            if (map[location[3 * i + 0] + t_x + (map_width / 2)][location[3 * i + 1] + t_y + (map_height / 2) - 1][location[3 * i + 2] + t_z + (map_width / 2)] == true)
            {
                return true;
            }
            if (location[3 * i + 1] + t_y == -(map_height / 2))
            {
                return true;
            }
        }
        return false;
    }

    void UpdateMap() // Updates blocks on a map after the block touching the floor.
    {
        for (int i = 0; i < 4; i++)
        {
            map[location[3 * i + 0] + t_x + (map_width / 2)][location[3 * i + 1] + t_y + (map_height / 2)][location[3 * i + 2] + t_z + (map_width / 2)] = true;
        }
    }
};

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
        eye(3, 3, 8),
        center(0, 0, 0),
        up(0, 1, 0),
        zoom_factor(6.0f),
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

bool show_wireframe = false;
bool show_grid = true;

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

Block block;


void idle()
{
    static clock_t prev_time = clock();
    clock_t curr_time = clock();
    if (1.0 * (curr_time - prev_time) / CLOCKS_PER_SEC > 1.0) {
        if (block.IsCollision() == true)
        {
            block.UpdateMap();
            FillCheck();
            if (IsGameEnd())
            {
                char ans;
                printf("Game is over and your game score is %d.\nDo you want another game? (y/n) ", score);
                std::cin >> ans;
                if (ans == 'y')
                {
                    block.init();
                    MapInit();
                    score = 0;
                }
                else
                {
                    exit(0);
                }
            }
            block.init();
        }
        block.Move(0, -1, 0);
        glutPostRedisplay();

        prev_time = curr_time;
    }
}

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
    block.init();
    block.Move(0, 0, 0);
    block.Rotation(1, 0, 0);

    glutIdleFunc(idle);

    glutDisplayFunc(display); // register call back function for the display event
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard); // register call back function for the keyboard event
    glutMouseWheelFunc(mouse_wheel);
    glutSpecialFunc(specialkey);
    glutMainLoop(); // enter the display event loop

}

void init()
{
    //srand(clock()); // to use random function
    program = build_program(); // build a program

    make_grid(5, 5, map_width, map_width);
    make_cube(1); //  get cube vertices position and register vertices information to VAO and VBO
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
        grid_mat = glm::translate(grid_mat, vec3(0, -(map_height / 2 + 0.5), 0));
        glUniform1i(4, 5);
        draw_grid(value_ptr(grid_mat));

        grid_mat = glm::translate(grid_mat, vec3(0, map_height, 0));
        draw_grid(value_ptr(grid_mat));
        glUniform1i(4, 0);
    }

    if (active_vao == 0) { // cube
        DrawMap();
        block.DrawBlock();

        //draw_cube(value_ptr(T));
    }
    glDisable(GL_POLYGON_OFFSET_FILL); // disable offset fill

    if (show_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // to draw vertices by line
        glLineWidth(1); // set line width
        glUniform1i(4, 1); // to set uniform variable where the location is 4
        if (active_vao == 0) { // cube
            draw_cube(value_ptr(T));
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
            mat4 Rx = rotate(mat4(1.0f), alpha * dy, vec3(transpose(V)[0]));
            mat4 Ry = rotate(mat4(1.0f), -alpha * dx, vec3(0, 1, 0));
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


void specialkey(int key, int x, int y)
{
    using namespace glm;
    mat4 VT = transpose(camera.get_viewing());
    vec3 x_axis(VT[0]); //vt[0]�� ����?
    x_axis[1] = 0;
    x_axis = glm::normalize(x_axis);
    float angle = (float)(180.0f * acosf(glm::dot(x_axis, vec3(1, 0, 0))) / M_PI);

    if (angle < 45)
    {
        switch (key)
        {
        case GLUT_KEY_LEFT:block.Move(-1, 0, 0); break;
        case GLUT_KEY_RIGHT:block.Move(+1, 0, 0); break;
        case GLUT_KEY_UP:block.Move(0, 0, -1); break;
        case GLUT_KEY_DOWN:block.Move(0, 0, +1); break;
        }
    }

    else if (angle < 135)
    {
        if (x_axis[2] > 0)
            switch (key)
            {
            case GLUT_KEY_LEFT:block.Move(0, 0, -1); break;
            case GLUT_KEY_RIGHT:block.Move(0, 0, +1); break;
            case GLUT_KEY_UP:block.Move(+1, 0, 0); break;
            case GLUT_KEY_DOWN:block.Move(-1, 0, 0); break;
            }
        else
        {
            switch (key)
            {
            case GLUT_KEY_LEFT:block.Move(0, 0, +1); break;
            case GLUT_KEY_RIGHT:block.Move(0, 0, -1); break;
            case GLUT_KEY_UP:block.Move(-1, 0, 0); break;
            case GLUT_KEY_DOWN:block.Move(+1, 0, 0); break;
            }
        }
    }
    else
    {
        switch (key)
        {
        case GLUT_KEY_LEFT:block.Move(+1, 0, 0); break;
        case GLUT_KEY_RIGHT:block.Move(-1, 0, 0); break;
        case GLUT_KEY_UP:block.Move(0, 0, +1); break;
        case GLUT_KEY_DOWN:block.Move(0, 0, -1); break;
        }
    }
}

void cb_menu(int value)
{
    if (value == 0) {
        camera.projection_mode = 0; // orthgonal
        camera.zoom_factor = 6.0f;
        glutPostRedisplay();
    }
    else {
        camera.projection_mode = 1; // perspective
        camera.zoom_factor = 4.0f;
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


void make_grid(GLfloat w, GLfloat h, int m, int n) {
    get_grid(vtx_pos[6], w, h, m, n); // get cube vertice positions
    get_color_3d_by_pos(vtx_clrs[6], vtx_pos[6], 0); // get colors
    glGenVertexArrays(1, &vao[6]); // generate VAO
    glBindVertexArray(vao[6]); // bind vao
    glGenBuffers(2, buffs[6]);  // generate VBOs
    bind_buffer(buffs[6][0], vtx_pos[6], program, "vPosition", 3); // assign vertice position data to "vPosition" variable which is in the program
    bind_buffer(buffs[6][1], vtx_clrs[6], program, "vColor", 3); // assign vertice position data to "vColor" variable which is in the program
}

void make_cube(GLfloat length) {
    get_box_3d(vtx_pos[0], length, length, length); // get cube vertice positions
    get_color_3d_by_pos(vtx_clrs[0], vtx_pos[0], 0); // get colors
    glGenVertexArrays(1, &vao[0]); // generate VAO
    glBindVertexArray(vao[0]); // bind vao
    glGenBuffers(2, buffs[0]);  // generate VBOs
    bind_buffer(buffs[0][0], vtx_pos[0], program, "vPosition", 3); // assign vertice position data to "vPosition" variable which is in the program
    bind_buffer(buffs[0][1], vtx_clrs[0], program, "vColor", 3); // assign vertice position data to "vColor" variable which is in the program
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w': show_wireframe = !show_wireframe; glutPostRedisplay(); break; // redraw display.
    case 'o': camera.projection_mode = 0; glutPostRedisplay(); break; // redraw display.
    case 'p': camera.projection_mode = 1; glutPostRedisplay(); break; // redraw display.

    case 'a': block.Rotation(1, 0, 0); glutPostRedisplay(); break;
    case 's': block.Rotation(0, 1, 0); glutPostRedisplay(); break;
    case 'd': block.Rotation(0, 0, 1); glutPostRedisplay(); break;

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
        { GL_VERTEX_SHADER, "term.vert" },
        { GL_FRAGMENT_SHADER, "term.frag" },
        { GL_NONE, NULL }
    };
    GLuint program = LoadShaders(shaders); // compile shaders and link program with compiled shaders 
    glUseProgram(program); // to use program
    return program;
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