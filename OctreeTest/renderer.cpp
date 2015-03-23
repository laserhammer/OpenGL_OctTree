/*
*	Class Renderer Author: Benjamin Robbins
*	Function: Maintains transforms of instances of meshes loaded into memory, draws, updates, initializes shaders
*	Dependencies: glm, requires vertex and fragment shaders
*/
#include "renderer.h"

Renderer::Renderer(int numInstances)
{
	_width = 800.0f;
	_height = 600.0f;
	_nearPlane = 1.0f;
	_farPlane = 10.0f;
	_viewRange = 45.0f;

	_eye = glm::vec3(2.5f, 2.5f, 2.5f);
	_at = glm::vec3(0.0f, 0.0f, 0.0f);
	_up = glm::vec3(0.0f, 0.0f, 1.0f);

	//Instancing
	_numInstances = numInstances;
	//Initialize transform arrays
	for(int i = 0; i < numInstances; i++)
	{
		_transforms.push_back(Transform());
		_transforms[i].rotation = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
		colliding.push_back(false);
	}
}

Renderer::~Renderer()
{
	glDeleteBuffers(1, &_vbo);
	glDeleteBuffers(1, &_vao);
	glDeleteBuffers(1, &_ebo);
	glDeleteProgram(_program.programPointer);
	glDeleteTextures(1, &_normMat.texturePointer);
	glDeleteTextures(1, &_collideMat.texturePointer);
}


void Renderer::init(const char* fshader, const char* vshader, GLfloat* normalTex, int widthN, int heightN, GLfloat* collideTex, int widthC, int heightC)
{
	//InitShader
	_program = ShaderProgram(fshader, vshader);
	glUseProgram(_program.programPointer);
	//Textures
	_normMat = Material(normalTex, GL_TEXTURE0, widthN, heightN);
	glUniform1i(glGetUniformLocation(_program.programPointer, "tex"), 0);
	_collideMat = Material(collideTex, GL_TEXTURE1, widthC, heightC);
	glUniform1i(glGetUniformLocation(_program.programPointer, "collideTex"), 1);
	//Get uniforms
	_uniModel = glGetUniformLocation(_program.programPointer, "model");
	_uniView = glGetUniformLocation(_program.programPointer, "view");
	_uniProj = glGetUniformLocation(_program.programPointer, "proj");
	_uniColliding = glGetUniformLocation(_program.programPointer, "colliding");
}

void Renderer::update(float time)
{
	for(int i = 0; i < _numInstances; i++)
	{
		_transforms[i].translate( _transforms[i].translate() * glm::translate(_transforms[i].velocity * time));
		_transforms[i].rotate = _transforms[i].rotate * glm::rotate(_transforms[i].rotation.w * time, glm::vec3(_transforms[i].rotation.x, _transforms[i].rotation.y, _transforms[i].rotation.z));
	}
}

void Renderer::display(GLint instance)
{
	updateInstance(instance,  (glm::mat4&)_transforms[instance].translate(), _transforms[instance].rotate, _transforms[instance].origin, _transforms[instance].scale);
	glUniform1f(_uniColliding, colliding[instance]);
}


Transform const& Renderer::transform(GLint instance)const
{
	return _transforms[instance];
}

Transform& Renderer::transform(GLint instance)
{
	return _transforms[instance];
}

void Renderer::updateInstance(GLint sphere, glm::mat4& trans, glm::mat4& rotate, glm::mat4& origin, glm::mat4& scale)
{
	_transforms[sphere].model = trans * (rotate * origin) * scale;
	_view = glm::lookAt(_eye, _at, _up);
	_proj = glm::perspective(_viewRange, _width / _height, _nearPlane, _farPlane);

	glUniformMatrix4fv(_uniModel, 1, GL_FALSE, glm::value_ptr(_transforms[sphere].model));
	glUniformMatrix4fv(_uniView, 1, GL_FALSE, glm::value_ptr(_view));
	glUniformMatrix4fv(_uniProj, 1, GL_FALSE, glm::value_ptr(_proj));
}