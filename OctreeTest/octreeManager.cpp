#include "octreeManager.h"
#include "sphereCollider.h"
#include "octree.h"

std::vector<Octree>* OctreeManager::_treeData = nullptr;
std::mutex OctreeManager::barrier[3];
std::lock_guard<std::mutex> OctreeManager::block(barrier[0]);

void OctreeManager::createOctree(glm::vec3 corner1, glm::vec3 corner2, SphereCollider* collider)
{
	barrier[0].unlock();
	barrier[1].unlock();
	barrier[2].unlock();
	_treeData = new std::vector<Octree>();
	int totalTreeObjs = 1;
	for (int i = 0; i < MAX_OCTREE_DEPTH + 1; ++i)
	{
		totalTreeObjs += 8 * pow(8, i);
	}
	_treeData->resize(totalTreeObjs);
	Octree* root = &(*_treeData)[0];
	root->id = 0;
	root->corner1 = corner1;
	root->corner2 = corner2;
	glm::vec3 c = (corner1 + corner2);
	root->center = glm::vec3(c.x / 2, c.y / 2, c.z / 2);
	root->depth = -1;
	root->hasChildren = false;
	root->colliderObj = collider;
	root->checkWalls();
	root->active = true;

	//_threads.reserve(4);
}

void OctreeManager::updateTree(float dt)
{
	Octree* root = &(*_treeData)[0];
	root->update(dt);
}

void OctreeManager::init()
{
	Octree* root = &(*_treeData)[0];
	root->init();
}

int OctreeManager::getChild(int parentID, int parentDepth, int childNum)
{
	return parentID + childNum * pow(8, parentDepth + 1);
}

Octree* OctreeManager::initChild(glm::vec3 c1, glm::vec3 c2, int parentDepth, SphereCollider* colliderObject, int parentID, std::map<Wall, bool>& parentWalls, glm::vec3& parentCenter, int childNum)
{
	Octree* newChild;
	int childID = getChild(parentID, parentDepth, childNum);
	newChild = &(*_treeData)[childID];

	//id is also the position in the array
	newChild->id = childID;
	//if this stuff is already set, skip it
	if (newChild->colliderObj != colliderObject)
	{
		newChild->corner1 = c1;
		newChild->corner2 = c2;
		glm::vec3 c = (c1 + c2);
		newChild->center = glm::vec3(c.x / 2, c.y / 2, c.z / 2);
		newChild->depth = parentDepth + 1;
		newChild->hasChildren = false;
		newChild->colliderObj = colliderObject;
		newChild->checkWalls(parentWalls, newChild->walls, parentCenter, newChild->corner1, newChild->corner2, false);
	}
	newChild->active = true;

	return newChild;
}

void OctreeManager::deactivateChildren(int parentID, int parentDepth)
{
	Octree* child;
	int childNum = 0;
	do
	{
		child = &((*_treeData)[getChild(parentID, parentDepth, ++childNum)]);
		child->active = false;
		child->colliders.clear();
	} while (childNum < 9);
}
/*
bool OctreeManager::launchThreadPotentialCollisions(void(Octree::*potentialCollisions)(std::vector<ColliderPair>& collisions), std::vector<ColliderPair>& collisions)
{
	bool threadAvailable = _threads.size() < THREADS;
	if (threadAvailable)
	{
		std::thread thread = std::thread(potentialCollisions, std::ref(collisions));
		_threads.push_back(thread);
		thread.detach();
	}

	return threadAvailable;
}
*/