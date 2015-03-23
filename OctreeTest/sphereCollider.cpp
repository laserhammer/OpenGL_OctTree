/*
*	Class SphereCollider Author: Benjamin Robbins
*	Function: Evaluates and resolves collisions between instances of Renderer objects
*	Dependencies: glm, Renderer, host instances for collision operations, Octree 
*/

#include "sphereCollider.h"
#include "octree.h"
#include "renderer.h"
#include <ppl.h>
using namespace concurrency;

SphereCollider::SphereCollider(GLfloat worldSize)
{
	_totalColliders = 0;
	_numTypes = 0;
	for(int i = 0; i < 6; i++)
	{
		if(i % 2 == 0)
		{
			_world[i] = worldSize;
		}
		else
		{
			_world[i] = -worldSize;
		}
	}
}
SphereCollider::~SphereCollider()
{
}

bool SphereCollider::testWallCollision(int collider, Wall wall)
{
	glm::vec3 dir = Octree::wallDirection(wall);
	//Check whether the ball is far enough in the "dir" direction, and whether
	//it is moving toward the wall
	Transform& currentTransform = _host[collider].host->transform(_host[collider].hostInstance);
	GLfloat colliderDirProj = glm::dot(_host[collider].collider, dir);
	GLfloat rad  = _host[collider].radius;
	GLfloat w = world(wall);
	return glm::dot(_host[collider].collider, dir) + _host[collider].radius > glm::abs(world(wall)) && 
		glm::dot(currentTransform.velocity, dir) > 0;
}

void SphereCollider::handleWallCollisions(std::vector<ColliderWallPair>& collisions)
{
	for(int i = 0; i < collisions.size(); i++)
	{
		Wall w = collisions[i].wall;
		GLint collider = collisions[i].collider;
		glm::vec3 vel = _host[collider].host->transform(_host[collider].hostInstance).velocity;
		switch (w)
		{
		case WALL_LEFT:
			if(_host[collider].collider.x - _host[collider].radius < _world[3])
			{
				_host[collider].collider.x = _world[3] + _host[collider].radius;
				vel.x *= -1.0f;
			}
			break;
		case WALL_RIGHT:
			if(_host[collider].collider.x + _host[collider].radius > _world[2])
			{
				_host[collider].collider.x = _world[2] - _host[collider].radius;
				vel.x *= -1.0f;
			}
			break;
		case WALL_FAR:
			if(_host[collider].collider.y + _host[collider].radius > _world[4])
			{
				_host[collider].collider.y = _world[4] - _host[collider].radius;
				vel.y *= -1.0f;
			}
			break;
		case WALL_NEAR:
			if(_host[collider].collider.y - _host[collider].radius < _world[5])
			{
				_host[collider].collider.y = _world[5] + _host[collider].radius;
				vel.y *= -1.0f;
			}
			break;
		case WALL_TOP:
			if(_host[collider].collider.z + _host[collider].radius > _world[0])
			{
				_host[collider].collider.z = _world[0] - _host[collider].radius;
				vel.z *= -1.0f;
			}
			break;
		case WALL_BOTTOM:
			if(_host[collider].collider.z - _host[collider].radius < _world[1])
			{
				_host[collider].collider.z = _world[1] + _host[collider].radius;
				vel.z *= -1.0f;
			}
			break;
		default:
			break;
		}
		_host[collider].host->transform(_host[collider].hostInstance).velocity = vel;
		_host[collider].host->transform(_host[collider].hostInstance).translate(glm::translate(glm::vec3(_host[collider].collider.x, _host[collider].collider.y, _host[collider].collider.z)));
	}
}

void SphereCollider::clearCollisions()
{
	int j = 0;
	int end = 0;
	for(int i = 0; i < _numTypes; i++)
	{
		end += _numColliders[i];
		for(j; j < end; j++)
		{
			_host[j].host->colliding[_host[j].hostInstance] = false;
			getMovement(j);
		}
	}
}

void SphereCollider::updateCollider()
{
	int j = 0;
	int end = 0;
	for(int i = 0; i < _numTypes; i++)
	{
		end += _numColliders[i];
		for(j; j < end; j++)
		{
			glm::vec3 hostPosition = _host[j].host->transform(_host[j].hostInstance).position();
			_host[j].collider = hostPosition + _host[j].colliderOffset;
		}
	}
}

void SphereCollider::resolveCollision(int colliderA, int colliderB, glm::vec3& normal, GLfloat minRadius, GLfloat distance)
{
	int instanceA = _host[colliderA].hostInstance;
	int instanceB = _host[colliderB].hostInstance;
	if(distance == 0)
	{
		return;
	}
	glm::vec3 normNormalized = normal / distance;

	//glm::mat4 translationA = glm::translate((normNormalized * minRadius) + _host[colliderB].host->transform(instanceB).position());

	//Translate a out of b to prevent trapping
	//_host[colliderA].host->transform(instanceA).translate(translationA);
	
	//Find the new velocities of colliders

	//Check to see if normal is already = to i
	if(normNormalized.x == 1.0f || normNormalized.x == -1.0f)
	{
		GLfloat tmp = _host[colliderA].host->transform(instanceA).velocity.x;
		_host[colliderA].host->transform(instanceA).velocity.x = _host[colliderB].host->transform(instanceB).velocity.x;
		_host[colliderB].host->transform(instanceB).velocity.x = tmp;
		return;
	}
	//Otherwise
	//Rotate normal to x-axis
	//Apply rotation to velocities
	glm::vec3 velA = _host[colliderA].host->transform(instanceA).velocity;
	glm::vec3 velB = _host[colliderB].host->transform(instanceB).velocity;
	//Theta: a dot b = mag(a) * mag(b) * cos(theta)
	//Theta = Acos(a.x)
	//Axis = a x b
	GLfloat thetaA = glm::acos(normNormalized.x) * 180 / 3.141592653589793238462688723;
	GLfloat thetaB = glm::acos(-normNormalized.x) * 180 / 3.141592653589793238462688723;

	glm::vec3 axisA = glm::cross(normNormalized, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 axisB = glm::cross(-normNormalized, glm::vec3(1.0f, 0.0f, 0.0f));

	glm::vec4 rotatedVelA = glm::rotate(thetaA, axisA) * glm::vec4(velA, 0.0f);
	glm::vec4 rotatedVelB = glm::rotate(thetaB, axisB) * glm::vec4(velB, 0.0f);

	//Reflect velocities assuming for now masses are equal
	glm::vec4 newVelA = glm::vec4(-rotatedVelB.x, rotatedVelA.y, rotatedVelA.z, 0.0f);
	glm::vec4 newVelB = glm::vec4(-rotatedVelA.x, rotatedVelB.y, rotatedVelB.z, 0.0f);

	//Un-rotate velocities
	glm::vec4 finalVelA = glm::rotate(-thetaA, axisA) * newVelA;
	glm::vec4 finalVelB = glm::rotate(-thetaB, axisB) * newVelB;

	//Apply new Velocities 
	_host[colliderA].host->transform(instanceA).velocity = glm::vec3(finalVelA);
	_host[colliderB].host->transform(instanceB).velocity = glm::vec3(finalVelB);

	updateCollider();
}

void SphereCollider::handleCollisions(std::vector<ColliderPair>& collisions)
{
	unsigned int size = collisions.size();
	parallel_for(size_t(0), size, [&](size_t i)
	{
		ColliderPair pair = collisions[i];

		int colliderA = pair.colliderA;
		int colliderB = pair.colliderB;
		std::vector<glm::vec3> colInfo = testCollision(colliderA, colliderB);
		if (colInfo.size() == 3)
		{
			Transform& transformA = _host[colliderA].host->transform(_host[colliderA].hostInstance);
			Transform& transformB = _host[colliderA].host->transform(_host[colliderA].hostInstance);

			//Reflect velocities
			resolveCollision(colliderA, colliderB, colInfo[1], colInfo[2][0], colInfo[2][1]);

			//Translate colliderA to prevent trapping
			transformA.translate(glm::translate(colInfo[0]));

		}
	});
	/*
	for(int i = 0; i < size; i++)
	{
		ColliderPair pair = collisions[i];

		int colliderA = pair.colliderA;
		int colliderB = pair.colliderB;
		std::vector<glm::vec3> colInfo = testCollision(colliderA, colliderB);
		if( colInfo.size() == 3)
		{
			Transform& transformA = _host[colliderA].host->transform(_host[colliderA].hostInstance);
			Transform& transformB = _host[colliderA].host->transform(_host[colliderA].hostInstance);

			//Reflect velocities
			resolveCollision(colliderA, colliderB, colInfo[1], colInfo[2][0], colInfo[2][1]);

			//Translate colliderA to prevent trapping
			transformA.translate(glm::translate(colInfo[0]));

		}
	}
	*/
	/*
	glm::vec3 normal = _host[colliderA].collider - _host[colliderB].collider;
	GLfloat minRadius = _host[colliderA].radius + _host[colliderB].radius;
	GLfloat mag = glm::length(normal);
	if(mag < minRadius)
	{
		_host[colliderA].host->colliding[_host[colliderA].hostInstance] = true;
		_host[colliderB].host->colliding[_host[colliderB].hostInstance] = true;
		resolveCollision(colliderA, colliderB, normal, minRadius, mag);

	}
	*/
}

std::vector<glm::vec3> SphereCollider::testCollision(int colliderA, int colliderB)
{
	std::vector<glm::vec3> re;
	glm::vec3 normal = _host[colliderA].collider - _host[colliderB].collider;
	GLfloat minRadius = _host[colliderA].radius + _host[colliderB].radius;
	GLfloat mag = glm::length(normal);
	if(mag == 0.0f)
		return re;
	if(mag < minRadius)
	{
		
		//Change color of collider for debugging
		_host[colliderA].host->colliding[_host[colliderA].hostInstance] = true;
		_host[colliderB].host->colliding[_host[colliderB].hostInstance] = true;
		//Translate colliderA out of colliderB
		//A
		re.push_back(glm::vec3((normal / mag * minRadius) + _host[colliderB].collider));
		//Normal
		re.push_back(normal);
		//Min radius and distance
		re.push_back(glm::vec3(minRadius, mag, 0.0f));
	}
	return re;
}

void SphereCollider::init(int numHosts, Renderer* hostObject, GLfloat radius)
{
	int currentColliders = _totalColliders;
	for(int i = currentColliders; i < numHosts + currentColliders; i++, _totalColliders++)
	{
		_host.push_back(Host());
		_host[i].colliderOffset = glm::vec3(0.0f, 0.0f, 0.0f);
		_host[i].radius = radius;
		_host[i].host = hostObject;
		_host[i].hostInstance = i - currentColliders;
	}
	_numTypes++;
	_numColliders.push_back(numHosts);
}

std::map<int, int>& SphereCollider::update(std::vector<ColliderPair>& collisions, std::vector<ColliderWallPair>& wallCollisions, GLfloat time)
{
	updateCollider();
	handleWallCollisions(wallCollisions);
	updateCollider();
	clearCollisions();
	handleCollisions(collisions);
	return movedColliders;
}

void SphereCollider::getMovement(int collider)
{
	glm::vec3& vel = _host[collider].host->transform(_host[collider].hostInstance).velocity;
	if(vel.x != 0 || vel.y != 0 || vel.z != 0)
		movedColliders[collider] = collider;
	else
		movedColliders[collider] = -1;
}

Host& SphereCollider::host(GLint i)
{
	return _host[i];
}
Host const& SphereCollider::host(GLint i)const
{
	return _host[i];
}

GLfloat const* SphereCollider::world()
{
	return _world;
}

GLfloat const& SphereCollider::world(Wall wall)const
{
	switch (wall) {
		case WALL_LEFT:
			return _world[3];
		case WALL_RIGHT:
			return _world[2];
		case WALL_FAR:
			return _world[4];
		case WALL_NEAR:
			return _world[5];
		case WALL_TOP:
			return _world[0];
		case WALL_BOTTOM:
			return _world[1];
		default:
			return 0.0f;
	}
}

int const& SphereCollider::totalColliders()const
{
	return _totalColliders;
}