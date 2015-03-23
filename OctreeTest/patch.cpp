/*
*	Class Patch Author: Benjamin Robbins
*	Parent class: Renderer
*	Function: Creates a patch in memory and applies perturbance based on control points
*	Note: Instancing is not possible with patches as each one has unique verticies
*	Dependencies: glm, Renderer, vertex and fragment shaders
*/

#include "patch.h"

#pragma region Constructor and Destructor
Patch::Patch()
	:Renderer()
{
	_vertCount = 0;
	_faceCount = 0;
}

Patch::~Patch()
{
	Renderer::~Renderer();
}

#pragma endregion

#pragma region Functions

void Patch::init(const char* fshader, const char* vshader, GLfloat* normalTex, int widthN, int heightN, GLfloat* collideTex, int widthC, int heightC)
{
	Renderer::init(fshader, vshader, normalTex, widthN, heightN, collideTex, widthC, heightC);

	generatePlane();

	//Create vertex buffer
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_verts), _verts, GL_DYNAMIC_DRAW);

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

void Patch::display()
{
	Renderer::display(0);
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, NUM_ELEMENTS, GL_UNSIGNED_INT, 0);
}

void Patch::berp()
{
	//Found this awsome algorithm @ http://web.cs.wpi.edu/~matt/courses/cs563/talks/surface/bez_surf.html
	// Precalculate values for Bernstein basis functions
	GLfloat steps = (float)NUM_VERTS;
	GLfloat uinc = 1.0f/(steps - 1);
	GLfloat u = 0.0f;

	GLfloat b_value[4][NUM_VERTS];	//Stores the Bernstein function values

	for(int i = 0; i < steps; i++, u += uinc)
	{
		GLfloat u_sqr = u * u;		//u^2
		GLfloat tmp = 1.0 - u;		//(1 - u)
		GLfloat tmp_sqr = tmp * tmp;//(1-u)^2

		b_value[0][i] = tmp * tmp_sqr;	//(1 - u)^2
		b_value[1][i] = 3 * u * tmp_sqr;//(3u(1 - u)^2
		b_value[2][i] = 3 * u_sqr * tmp;//3u^2(1 - u)
		b_value[3][i] = u * u_sqr;		//u^3
	}

	//Use blending tables to calculate points in "row curve"
	glm::vec3 newControlPoints[4];
	for(int i = 0; i < NUM_VERTS; i++)
	{
		//Blend the four rows together to get column at this point on the surface
		newControlPoints[0] = b_value[0][i] * _controlPoints[0] + b_value[1][i] * _controlPoints[1] + b_value[2][i] * _controlPoints[2] + b_value[3][i] * _controlPoints[3];
		newControlPoints[1] = b_value[0][i] * _controlPoints[4] + b_value[1][i] * _controlPoints[5] + b_value[2][i] * _controlPoints[6] + b_value[3][i] * _controlPoints[7];
		newControlPoints[2] = b_value[0][i] * _controlPoints[8] + b_value[1][i] * _controlPoints[9] + b_value[2][i] * _controlPoints[10] + b_value[3][i] * _controlPoints[11];
		newControlPoints[3] = b_value[0][i] * _controlPoints[12] + b_value[1][i] * _controlPoints[13] + b_value[2][i] * _controlPoints[14] + b_value[3][i] * _controlPoints[15];
		for(int j = 0; j < NUM_VERTS; j++)
		{
			//blend the single column to get this vertex
			glm::vec3 newPoint = b_value[0][j] * newControlPoints[0] + b_value[1][j] * newControlPoints[1]  + b_value[2][j] * newControlPoints[2] + b_value[3][j] * newControlPoints[3];
			_verts[(j + (i * NUM_VERTS)) * 8] = newPoint.x;
			_verts[(j + (i * NUM_VERTS)) * 8 + 1] = newPoint.y;
			_verts[(j + (i * NUM_VERTS)) * 8 + 2] = newPoint.z;
		}
	}
	//Send new array data to buffer to be rendered
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_verts), _verts, GL_DYNAMIC_DRAW);
}

void Patch::addFace(GLint a, GLint b, GLint c)
{
	_elements[_faceCount * 3] = a;
	_elements[_faceCount * 3 + 1] = b;
	_elements[_faceCount * 3 + 2] = c;
	_faceCount++;
}

void Patch::addVert(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b, GLfloat u, GLfloat v)
{
	_verts[_vertCount * 8] = x;
	_verts[_vertCount * 8 + 1] = y;
	_verts[_vertCount * 8 + 2]  = z;
	_verts[_vertCount * 8 + 3] = r;
	_verts[_vertCount * 8 + 4] = g;
	_verts[_vertCount * 8 + 5] = b;
	_verts[_vertCount * 8 + 6] = u;
	_verts[_vertCount * 8 + 7] = v;
	_vertCount++;
}

void Patch::generatePlane()
{
	//Generate the vertices for the plane
	for(int i = 0; i < NUM_VERTS; i++)
	{
		for(int j = 0; j < NUM_VERTS; j++)
		{
			addVert(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, (GLfloat)i / (NUM_VERTS - 1), (GLfloat)j / (NUM_VERTS - 1));
		}
	}
	//Add 4 rows of control points
	int cp = 0;
	float yOffset = 1.0f / 3.0f;
	float xOffset = 1.0f / 3.0f;
	glm::vec3 baseVec = glm::vec3();
	for(int row = 0; row < 4; row++)
	{
		_controlPoints[cp] = glm::vec3(baseVec.x, baseVec.y + yOffset * row, baseVec.z);
		cp++;
		_controlPoints[cp] = glm::vec3(baseVec.x + xOffset, baseVec.y + yOffset * row, baseVec.z);
		cp++;
		_controlPoints[cp] = glm::vec3(baseVec.x + xOffset * 2, baseVec.y + yOffset * row, baseVec.z);
		cp++;
		_controlPoints[cp] = glm::vec3(baseVec.x + xOffset * 3, baseVec.y + yOffset * row, baseVec.z);
		cp++;
	}
	//Generate faces out of Verts
	for(int i = 0; i < NUM_VERTS * (NUM_VERTS - 1); i += NUM_VERTS)
	{
		for(int j = 0; j < NUM_VERTS - 1; j += 1)
		{
			addFace(i + j, i + j + 1, i + NUM_VERTS + j);
			addFace(i + j + 1, i + NUM_VERTS + j + 1, i + NUM_VERTS + j);

			//Reverse Face
			addFace(i + NUM_VERTS + j, i + j + 1, i + j);
			addFace(i + NUM_VERTS + j, i + NUM_VERTS + j + 1, i + j + 1);
		}
	}
}

#pragma endregion

#pragma region getters and setters

glm::vec3& Patch::operator [](int i)
{
	if(i < 0 || i > 15)
		std::cout << "Array is out of bounds\n";
	else
		return _controlPoints[i];
}
glm::vec3 Patch::operator [](int i) const
{
	if(i < 0 || i > 15)
		std::cout << "Array is out of bounds\n";
	else
		return _controlPoints[i];
}

#pragma endregion