#ifndef ICOSPHERE_H
#define ICOSPHERE_H
#include "renderer.h"

class Icosphere : public Renderer
{
public:
	explicit Icosphere(int numSpheres = 1);
	~Icosphere();

	
	void init(const char* fshader, const char* vshader, GLfloat* normalTex, int widthN, int heightN, GLfloat* collideTex, int widthC, int heightC);
	void display(GLint sphere);

private:

	void addVert(GLfloat x, GLfloat y, GLfloat z, int index);
	void addEle(GLint a, GLint b, GLint c, int index);
	void splitFace(GLint a, GLint b, GLint c, int index, GLint*& tmpArray);
	int getMiddleVert(GLint p1, GLint p2);
	void generateIcosphere();

	//---Icosphere specific
	GLfloat _verts[1296];
	GLint _elements[960];//20original faces * 3 verts for each face * 4 one refinement * 4 one refinement = 960

	std::map<GLint64, GLint> _middlePointIndexCache;

	GLint _faceCount;

	GLint _vertCount;

};


#endif