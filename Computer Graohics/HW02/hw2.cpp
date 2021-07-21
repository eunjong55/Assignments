//HW2 21700556 Lee, EunJong

#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "LoadShaders.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#define VSET2(v,a,b) do {(v)[0]=(a); (v)[1]=(b);} while(0)
#define VSET2PP(v,a,b) do {VSET2(v,a,b); v+=2;} while(0)

typedef std::vector<GLfloat> GLvec;

GLuint VertexArrays[3];
GLuint Buffers[2][2];

GLuint ColorBuffer; // only one buffer for color.

GLuint active_vao = 0;
GLvec vertices[2];
GLvec colors[3];

int build_program();
void init();
void keyboard(unsigned char key, int x, int y);
void display();
void get_circle_2d(GLvec& p, int sectors, GLfloat radius);
void get_rect_2d(GLvec& p, GLfloat width, GLfloat height);
void get_vertex_color(GLvec& color, GLuint n, GLfloat r, GLfloat g, GLfloat b);
void bind_buffer(GLint buffer, GLvec& vec, int program, const GLchar* attri_name, GLint attri_size);
void share_buffer(GLint buffer, int program, const GLchar* attri_name, GLint attri_size);

void main(int argc, char** argv)
{
	glutInit(&argc, argv); // initialization
	glutInitDisplayMode(GLUT_RGBA); // set display mode
	glutInitWindowSize(512, 512); // set window size
	glutCreateWindow("VAO example"); // set window name and create window
	GLenum err = glewInit();
	if (err != GLEW_OK) // error handler
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	init();
	glutDisplayFunc(display); // regisiter display function
	glutKeyboardFunc(keyboard); // register keyboard function
	glutMainLoop(); // go loop
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case '1': active_vao = 0; glutPostRedisplay(); printf("Key '%d'\n", 1);  break; // redraw display.
	case '2': active_vao = 1; glutPostRedisplay(); printf("Key '%d'\n", 2); break;
	case '3': active_vao = 2; glutPostRedisplay(); printf("Key '%d'\n", 3); break;
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT); // clear the buffer.
	glBindVertexArray(VertexArrays[active_vao]); // choose vao to use.
	if (active_vao == 2) glDrawArrays(GL_TRIANGLE_FAN, 0, vertices[0].size() / 2); // draw something which is in buffer.
	else glDrawArrays(active_vao == 0 ? GL_TRIANGLE_FAN : GL_TRIANGLES, 0, vertices[active_vao].size() / 2);
	glFlush(); // immediate application
}

void init()
{
	int program = build_program(); // compile.
	get_circle_2d(vertices[0], 50, 0.5f); // get circle position.
	get_rect_2d(vertices[1], 1.0f, 0.8f); // get rect position.
	get_vertex_color(colors[0], vertices[0].size() / 2, 0.8f, 0.2f, 0.5f); // get color.
	get_vertex_color(colors[1], vertices[1].size() / 2, 0.9f, 0.9f, 0.1f);
	get_vertex_color(colors[2], vertices[0].size() / 2, 0.3f, 0.3f, 1.0f);

	glGenVertexArrays(3, VertexArrays);
	for (int i = 0; i < 3; ++i)
	{
		glBindVertexArray(VertexArrays[i]); // create vao in server and return it's id.

		if (i < 2) // use 2 buffers.
		{
			glGenBuffers(2, Buffers[i]); // generate 2 buffer
			bind_buffer(Buffers[i][0], vertices[i], program, "vPosition", 2); // register position to program.
			bind_buffer(Buffers[i][1], colors[i], program, "vColor", 3); // register color to program.
		}
		else // use only color buffer/
		{
			glGenBuffers(1, &ColorBuffer); // generate 1 buffer
			share_buffer(Buffers[0][0], program, "vPosition", 2);
			bind_buffer(ColorBuffer, colors[i], program, "vColor", 3);
		}

	}
}

int build_program()
{
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "vao_example.vert" },
		{ GL_FRAGMENT_SHADER, "vao_example.frag" },
		{ GL_NONE, NULL }
	};
	GLuint program = LoadShaders(shaders);
	glUseProgram(program);
	return program;
}

void get_circle_2d(GLvec& p, int sectors, GLfloat radius) // get circle position.
{
	p.resize((sectors + 2) * 2);
	GLfloat* data = p.data();

	VSET2PP(data, 0, 0);

	for (int i = 0; i <= sectors; ++i)
	{
		GLfloat phi = (GLfloat)(2 * M_PI * i / sectors);
		GLfloat x = radius * cos(phi);
		GLfloat y = radius * sin(phi);
		VSET2PP(data, x, y);
	}
}

void get_rect_2d(GLvec& p, GLfloat width, GLfloat height) // get rect position.
{
	GLfloat w2 = width / 2;
	GLfloat h2 = height / 2;

	p.resize(12);
	GLfloat* data = p.data();

	VSET2PP(data, -w2, -h2);
	VSET2PP(data, +w2, -h2);
	VSET2PP(data, -w2, +h2);

	VSET2PP(data, +w2, -h2);
	VSET2PP(data, +w2, +h2);
	VSET2PP(data, -w2, +h2);
}

void get_vertex_color(GLvec& color, GLuint n, GLfloat r, GLfloat g, GLfloat b) // get vertex color.
{
	color.resize(n * 3);
	for (GLuint i = 0; i < n; ++i)
	{
		color[i * 3 + 0] = r;
		color[i * 3 + 1] = g;
		color[i * 3 + 2] = b;
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

void share_buffer(GLint buffer, int program, const GLchar* attri_name, GLint attri_size)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	// already data is allocated, so we skip allocating data.
	GLint location = glGetAttribLocation(program, attri_name);
	glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(location);
}