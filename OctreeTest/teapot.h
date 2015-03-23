#ifndef TEAPOT_H
#define TEAPOT_H
#include "utils.h"
#include "renderer.h"
class Patch;
class Teapot : public Renderer
{
public:
	Teapot(int numTeapots = 1);
	~Teapot();

	//Functions
	void init(const char* fshader, const char* vshader, GLfloat* normalTex, int widthN, int heightN, GLfloat* collideTex, int widthC, int heightC);
	void display(GLint teapot);
	void update(GLfloat time);

private:

	static const int NUM_PATCHES = 28;
	Patch* _patches[NUM_PATCHES];

	GLint _numTeapots;

	GLfloat _time;
};
#endif