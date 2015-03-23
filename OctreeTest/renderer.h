#ifndef RENDERER_H
#define RENDERER_H
#include "utils.h"
#include "init_shader.h"

struct Transform
{
	glm::mat4 model;
	glm::mat4 rotate;
	glm::mat4 scale;
	glm::mat4 origin;
	glm::vec4 rotation;
	glm::vec3 velocity;

	//When translate is modified, so is position
	//read only
	glm::mat4 const& translate() const
	{
		return _translate;
	}
	//read/write
	glm::mat4& translate(glm::mat4& value)
	{
		_translate = value;
		_position = glm::vec3(value[3].x, value[3].y, value[3].z);
		return _translate;
	}
	//Position is read-only
	glm::vec3 const& position() const
	{
		return _position;
	}
private:
	glm::mat4 _translate;
	glm::vec3 _position;
};

struct ShaderProgram
{
	GLuint programPointer;
	ShaderProgram(const char* fshader, const char* vshader)
	{
		char* shaders[] = {"fshader.glsl", "vshader.glsl"};
		GLenum types[] = {GL_FRAGMENT_SHADER, GL_VERTEX_SHADER};
		int numShaders = 2;

		programPointer = initShaders(shaders, types, numShaders);
	}
	ShaderProgram()
	{
		programPointer = 0;
	}
};

struct Material
{
	GLuint texturePointer;
	GLenum activeTexture;
	Material(GLfloat* pixels = nullptr, GLenum activeTexture = 0, int width = 0, int height = 0)
	{
		this->activeTexture = activeTexture;
		glGenTextures(1, &texturePointer);
		glActiveTexture(activeTexture);
		glBindTexture(GL_TEXTURE_2D, texturePointer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

};

class Renderer
{
public:

	explicit Renderer(int numInstances = 1);
	~Renderer();

	void init(const char* fshader, const char* vshader, GLfloat* normalTex, int widthN, int heightN, GLfloat* collideTex, int widthC, int heightC);
	void update(float time);
	void display(GLint instance);

	Transform const& transform(GLint instance)const;
	Transform& transform(GLint instance);

	std::vector<bool> colliding;

protected:
	void updateInstance(GLint instance, glm::mat4& trans, glm::mat4& rotate, glm::mat4& origin, glm::mat4& scale);

	//---Rendering
	ShaderProgram _program;
	//material
	Material _normMat, _collideMat;
	GLint _uniColliding;

	//Data storage
	GLuint _vbo, _vao, _ebo;

	//Model, View, Projection
	GLfloat _width, _height, _nearPlane, _farPlane, _viewRange;

	glm::vec3 _eye, _at, _up;

	GLint _uniModel, _uniView, _uniProj;

	glm::mat4 _view, _proj;

	std::vector<Transform> _transforms;

	//--Instancing
	GLint _numInstances;
};
#endif