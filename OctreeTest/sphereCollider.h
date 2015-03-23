#ifndef SPHERE_COLLIDER
#define SPHERE_COLLIDER
#include "utils.h"
struct ColliderPair;
struct ColliderWallPair;
enum Wall;
class Renderer;
struct Host
{
	glm::vec3 collider;
	glm::vec3 colliderOffset;
	GLfloat radius;
	Renderer* host;
	GLint hostInstance;
};

class SphereCollider
{
public:
	SphereCollider(GLfloat worldSize = 1.0f);
	~SphereCollider();

	void handleWallCollisions(std::vector<ColliderWallPair>& collisions);
	bool testWallCollision(int collider, Wall wall);
	void clearCollisions();
	void updateCollider();
	void resolveCollision(int colliderA, int colliderB, glm::vec3& normal, GLfloat minRadius, GLfloat distance);
	void handleCollisions(std::vector<ColliderPair>& collisions);
	std::vector<glm::vec3> testCollision(int colliderA, int colliderB);
	void init(int numHosts, Renderer* hostObject, GLfloat radius);
	std::map<int, int>& update(std::vector<ColliderPair>& collisions, std::vector<ColliderWallPair>& wallCollisions, GLfloat time);
	void getMovement(int collider);

	Host& host(GLint i);
	Host const& host(GLint i)const;

	GLfloat const* world();
	GLfloat const& world(Wall wall)const;

	int const& totalColliders()const;

private:
	
	std::vector<Host> _host;
	GLint _numTypes;
	std::vector<int> _numColliders;
	GLint _totalColliders;
	GLfloat _world[6];
	std::map<int, int> movedColliders;
};

#endif