#ifndef PATCH_H
#define PATHC_H
#include "utils.h"
#include "renderer.h"
class Patch :public Renderer
{
public:
#pragma region Constructor etc

	Patch();
	~Patch();

#pragma endregion

#pragma region Functions

	void init(const char* fshader, const char* vshader, GLfloat* normalTex, int widthN, int heightN, GLfloat* collideTex, int widthC, int heightC);
	void display();
	void berp();
	void addFace(GLint a, GLint b, GLint c);
	void addVert(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b, GLfloat u, GLfloat v);
	void generatePlane();

#pragma endregion

#pragma region getters and setters

	//Control Points
	glm::vec3& operator [](int i);
	glm::vec3 operator [](int i) const;

#pragma endregion

protected:

	static const int NUM_VERTS = 10;
	static const int NUM_VERTS_STORED = NUM_VERTS * NUM_VERTS * 8;
	static const int NUM_ELEMENTS = (NUM_VERTS - 1) * (NUM_VERTS - 1) * 12;

	//Geometry
	GLfloat _verts[NUM_VERTS_STORED];	
	GLuint _elements[NUM_ELEMENTS];	//100 quads * 2 triangles * 3 verts * 2 for reverse side

	GLint _vertCount, _faceCount;


	glm::vec3 _controlPoints[16];

};

#endif