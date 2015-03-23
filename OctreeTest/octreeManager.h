#pragma once
#include <glm\glm.hpp>
#include <vector>
#include <map>
#include <ppl.h>
#include <mutex>

using namespace concurrency;

#define THREADS 4;

enum Wall;
struct ColliderPair;
class SphereCollider;
class Octree;
class OctreeManager
{
public:
	static void createOctree(glm::vec3 corner1, glm::vec3 corner2, SphereCollider* collider);

	static void updateTree(float dt);

	static void init();

	static int getChild(int parentID, int parentDepth, int childNum);

	static Octree* initChild(glm::vec3 c1, glm::vec3 c2, int parentDepth, SphereCollider* colliderObject, int parentID, std::map<Wall, bool>& parentWalls, glm::vec3& parentCenter, int childNum);

	static void deactivateChildren(int parentID, int parentDepth);

	static void activateChild(int parentDepth, int parentID, int childNumber);

	static std::mutex barrier[3];
	static std::lock_guard<std::mutex> block;

private:

	static std::vector<Octree>* _treeData;
};

