/*
*	Class Icosphere Author: Benjamin Robbins
*	Parent class: Renderer
*	Function: Creates an icosphere in memory
*	Dependencies: glm, Renderer, requires vertex and fragment shaders
*/
#include "icosphere.h"


Icosphere::Icosphere(int numSpheres)
	:Renderer(numSpheres)
{
	_faceCount = 0;

	_vertCount = 0;
}

Icosphere::~Icosphere()
{
	Renderer::~Renderer();
}

void Icosphere::addVert(GLfloat x, GLfloat y, GLfloat z, int index)
{
	GLfloat length = sqrt(x * x + y * y + z * z);
	//Position
	_verts[index * 8] = x / length;
	_verts[index * 8 + 1] = y / length;
	_verts[index * 8 + 2] = z / length;
	//Color
	_verts[index * 8 + 3] = 1.0f;
	_verts[index * 8 + 4] = 1.0f;
	_verts[index * 8 + 5] = 1.0f;
	//UV
	_verts[index * 8 + 6] = (glm::asin(x /length )/ glm::pi<GLfloat>() + 0.5);
	_verts[index * 8 + 7] = (glm::asin(y /length )/ glm::pi<GLfloat>() + 0.5);
	_vertCount++;
}

void Icosphere::addEle(GLint a, GLint b, GLint c, int index)
{
	_elements[index * 3] = a;
	_elements[index * 3 + 1] = b;
	_elements[index * 3 + 2] = c;
	_faceCount++;
}

void Icosphere::splitFace(GLint a, GLint b, GLint c, int index, GLint*& tmpArray)
{
	tmpArray[index * 3] = a;
	tmpArray[index * 3 + 1] = b;
	tmpArray[index * 3 + 2] = c;
	_faceCount++;
}

int Icosphere::getMiddleVert(GLint p1, GLint p2)
{
	//first check if we have it already
	bool firstIsSmaller = p1 < p2;
	GLint64 smallerIndex = firstIsSmaller ? p1 : p2;
	GLint64 greaterIndex = firstIsSmaller ? p2 : p1;
	GLint64 key = (smallerIndex << 32) + greaterIndex;
	
	GLint ret;
	if(ret = _middlePointIndexCache[key])
	{
		return ret;
	}

	glm::vec3 vert1 = glm::vec3(_verts[p1 * 8], _verts[p1 * 8 + 1], _verts[p1 * 8 + 2]);
	glm::vec3 vert2 = glm::vec3(_verts[p2 * 8], _verts[p2 * 8 + 1], _verts[p2 * 8 + 2]);
	ret = _vertCount;

	//Calculate the position of this new vertex
	addVert(
		(vert1.x + vert2.x) / 2.0f, 
		(vert1.y + vert2.y) / 2.0f,
		(vert1.z + vert2.z) / 2.0f,
		_vertCount
		);
	_middlePointIndexCache[key] = ret;
	return ret;
}

void Icosphere::generateIcosphere()
{
	//Create 12 Verts of an icosahedron
	GLfloat t = (1.0f + sqrt(5.0f)) / 2.0f;

	addVert(-1.0f,  t, 0.0f, 0);
	addVert( 1.0f,  t, 0.0f, 1);
	addVert(-1.0f, -t, 0.0f, 2);
	addVert( 1.0f, -t, 0.0f, 3);

	addVert( 0.0f, -1.0f,  t, 4);
	addVert( 0.0f,  1.0f,  t, 5);
	addVert( 0.0f, -1.0f, -t, 6);
	addVert( 0.0f,  1.0f, -t, 7);

	addVert( t, 0.0f, -1.0f, 8);
	addVert( t, 0.0f,  1.0f, 9);
	addVert(-t, 0.0f, -1.0f, 10);
	addVert(-t, 0.0f,  1.0f, 11);

	//Create 20 triangles of icosahedron

	//5 faces around point 0
	addEle(0, 11, 5, 0);
	addEle(0, 5, 1, 1);
	addEle(0, 1, 7, 2);
	addEle(0, 7, 10, 3);
	addEle(0, 10, 11, 4);

	//5 adjacent faces
	addEle(1, 5, 9, 5);
	addEle(5, 11, 4, 6);
	addEle(11, 10, 2, 7);
	addEle(10, 7,  6, 8);
	addEle(7, 1, 8, 9);

	//5 faces around point 3
	addEle(3, 9, 4, 10);
	addEle(3, 4, 2, 11);
	addEle(3, 2, 6, 12);
	addEle(3, 6, 8, 13);
	addEle(3, 8, 9, 14);

	//5 adjacent faces
	addEle(4, 9, 5, 15);
	addEle(2, 4, 11, 16);
	addEle(6, 2, 10, 17);
	addEle(8, 6, 7, 18);
	addEle(9, 8, 1, 19);

	//Refine into sphere
	
	for( int i = 0; i < 2; i++)
	{
		GLint currentFaces = _faceCount;
		GLint newFaces[960] = {0};
		_faceCount = 0;
		for(int j = 0; j < currentFaces; j++)
		{
			int a = getMiddleVert(_elements[j * 3], _elements[j * 3 + 1]);
			int b = getMiddleVert(_elements[j * 3 + 1], _elements[j * 3 + 2]);
			int c = getMiddleVert(_elements[j * 3 + 2], _elements[j * 3]);

			newFaces[_faceCount * 3] = _elements[j * 3];
			newFaces[_faceCount * 3 + 1] = a;
			newFaces[_faceCount * 3 + 2] = c;
			_faceCount++;

			newFaces[_faceCount * 3] = _elements[j * 3 + 1];
			newFaces[_faceCount * 3 + 1] = b;
			newFaces[_faceCount * 3 + 2] = a;
			_faceCount++;

			newFaces[_faceCount * 3] = _elements[j * 3 + 2];
			newFaces[_faceCount * 3 + 1] = c;
			newFaces[_faceCount * 3 + 2] = b;
			_faceCount++;

			newFaces[_faceCount * 3] = a;
			newFaces[_faceCount * 3 + 1] = b;
			newFaces[_faceCount * 3 + 2] = c;
			_faceCount++;
		}
		for(int i = 0 ; i < 960; i++)
		{
			_elements[i] = newFaces[i];
		}
	}
}

void Icosphere::init(const char* fshader, const char* vshader, GLfloat* normalTex, int widthN, int heightN, GLfloat* collideTex, int widthC, int heightC)
{
	Renderer::init(fshader, vshader, normalTex, widthN, heightN, collideTex, widthC, heightC);

	generateIcosphere();

	//Create vertex buffer
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_verts), _verts, GL_STATIC_DRAW);

	//Create array buffer
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	//Create elements buffer
	glGenBuffers(1, &_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_elements), _elements, GL_STATIC_DRAW);

	//Specify layout of point data
	GLint posAttrib = glGetAttribLocation(_program.programPointer, "pos");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);

	GLint colAttrib = glGetAttribLocation(_program.programPointer, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

	GLint texAttrib = glGetAttribLocation(_program.programPointer, "uv");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

}

void Icosphere::display(GLint sphere)
{
	Renderer::display(sphere);
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, 960, GL_UNSIGNED_INT, 0);
}

