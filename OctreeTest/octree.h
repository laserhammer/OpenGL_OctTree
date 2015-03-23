// based off of the Octree example given in class
// Collision evaluation functionality has been stripped
// Gravity application functionality has been stripped
// Modified to use indicies of instances of
// a Collider object rather than its own ball objects
#ifndef OCTREE_H
#define OCTREE_H

#include "utils.h"
#include "sphereCollider.h"

enum Wall {WALL_LEFT, WALL_RIGHT, WALL_FAR, WALL_NEAR, WALL_TOP, WALL_BOTTOM};

struct ColliderPair {
	GLint colliderA;
	GLint colliderB;	
};

struct ColliderWallPair {
	GLint collider;
	Wall wall;
};

const int MAX_OCTREE_DEPTH = 4;
const int MIN_COLLS_PER_OCTREE = 3;
const int MAX_COLLS_PER_OCTREE = 6;

class Octree
{
public:

	//Octree(glm::vec3 c1, glm::vec3 c2, int parentDepth, SphereCollider* colliderObject, int parentId, std::map<Wall, bool>& parentWalls, glm::vec3& parentCenter);
	Octree();
	~Octree(void);

	void checkWalls();
	void checkWalls(std::map<Wall, bool>& parentWalls, std::map<Wall, bool>& _walls, glm::vec3& parentCenter, glm::vec3& corner1, glm::vec3& corner2, bool root);
	void init();
	void add(GLint collider, bool toParent);
	void potentialCollisions(std::vector<ColliderPair>& collisions);
	void collideWithParent(std::vector<ColliderPair>& collisions, GLint collider);
	void potentialWallCollisions(std::vector<ColliderWallPair>& collisions);
	void update(GLfloat time);
	static glm::vec3 wallDirection(Wall wall);

	bool active;

	glm::vec3 corner1; //(minX, minY, minZ)
	glm::vec3 corner2; //(maxX, maxY, maxZ)
	glm::vec3 center;//((minX + maxX) / 2, (minY + maxY) / 2, (minZ + maxZ) / 2)

	bool hasChildren;

	std::vector<GLint> colliders;

	int depth;
	//int _numColliders;

	SphereCollider* colliderObj;

	int id;

	std::map<Wall, bool> walls;

private:
	Octree* _children[2][2][2];

	//Internal Functions
	void fileCollider(GLint collider, bool addColl, bool storeInThis);
	
	void haveChildren();

	void collectBalls(std::vector<GLint> &colliders);

	void destroyChildren();

	void potentialWallCollisions(std::vector<ColliderWallPair> &colliderWallPairs, Wall w, char coord, int dir);
	
	void moveColliders(std::map<GLint, GLint> &colliders);

	bool checkChild(GLint collider);

	void addAll(std::map<GLint, GLint>& colliders);

	void addAll(std::vector<GLint>& colliders);

	void removeAll(std::map<GLint, GLint>& colliders);

	glm::vec3 getColliderPos(GLint collider);

	GLfloat getColliderRad(GLint collider);
};

#endif

