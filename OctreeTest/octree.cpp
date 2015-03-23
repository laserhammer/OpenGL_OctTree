/*
*	Based on the octree example given in class
*	Author: Jesse Charland, Ben Robbins
*	Function: Organizes collider instances into an
*	octree to create more efficient collision detection
*	Dependencies: sphereCollider, glm
*/
#include "octree.h"
#include "renderer.h"
#include "octreeManager.h"
#include <ppl.h>
#include <thread>
#include <mutex>

using namespace concurrency;

Octree::Octree()
{
	active = false;
	colliders.reserve(8);
}

Octree::~Octree(void)
{
	if(hasChildren)
	{
		destroyChildren();
	}
}

void Octree::checkWalls()
{
	checkWalls(walls, walls, corner1, corner1, corner2, true);
}

void Octree::init()
{
	for(int i = 0; i < colliderObj->totalColliders(); i++)
	{
		add(i, false);
	}
}

void Octree::add(GLint collider, bool toParent)
{
	//_numColliders++;
	if(!hasChildren && depth < MAX_OCTREE_DEPTH && colliders.size() + 1 > MAX_COLLS_PER_OCTREE && checkChild(collider) && !toParent)
	{
		haveChildren();
	}

	if(hasChildren && !toParent)
	{
		fileCollider(collider, true, true);
	}
	else
	{
		colliders.push_back(collider);
	}
}

void Octree::potentialCollisions(std::vector<ColliderPair> &collisions)
{
	
	if(hasChildren)
	{
		//If the tree has children and colliders, then the colliders need to 
		//be paired with all colliders in children and each other
		int size = colliders.size();
		if(size > 0)
		{
			//std::cout << "Child " << ChildNumber << " has " << _colliders.size() << " colliders\n";
			for(int l = 0; l < size; l++)
			{
				GLint colliderA = (colliders)[l];
				_children[0][0][0]->collideWithParent(collisions, colliderA);
				_children[0][0][1]->collideWithParent(collisions, colliderA);
				_children[0][1][0]->collideWithParent(collisions, colliderA);
				_children[0][1][1]->collideWithParent(collisions, colliderA);
				_children[1][0][0]->collideWithParent(collisions, colliderA);
				_children[1][0][1]->collideWithParent(collisions, colliderA);
				_children[1][1][0]->collideWithParent(collisions, colliderA);
				_children[1][1][1]->collideWithParent(collisions, colliderA);

				while (!OctreeManager::barrier[0].try_lock());

				int size = colliders.size();
				for(int j = l + 1; j < size; j++)
				{
					GLint colliderB = (colliders)[j];
					ColliderPair cp;
					cp.colliderA = colliderA;
					cp.colliderB = colliderB;
					collisions.push_back(cp);
				}

				OctreeManager::barrier[0].unlock();
			}
		}
		//Recurse into children
		
		if (this->depth == -1)
		{	
			Octree *(*children)[2][2] = _children;
			parallel_invoke(
				[&collisions, &children] { children[0][0][1]->potentialCollisions(collisions); },
				[&collisions, &children] { children[0][0][0]->potentialCollisions(collisions); },
				[&collisions, &children] { children[0][1][1]->potentialCollisions(collisions); },
				[&collisions, &children] { children[0][1][0]->potentialCollisions(collisions); },
				[&collisions, &children] { children[1][0][1]->potentialCollisions(collisions); },
				[&collisions, &children] { children[1][0][0]->potentialCollisions(collisions); },
				[&collisions, &children] { children[1][1][1]->potentialCollisions(collisions); },
				[&collisions, &children] { children[1][1][0]->potentialCollisions(collisions); }
			);
		}
		else
		{
			_children[0][0][0]->potentialCollisions(collisions);
			_children[0][0][1]->potentialCollisions(collisions);
			_children[0][1][0]->potentialCollisions(collisions);
			_children[0][1][1]->potentialCollisions(collisions);
			_children[1][0][0]->potentialCollisions(collisions);
			_children[1][0][1]->potentialCollisions(collisions);
			_children[1][1][0]->potentialCollisions(collisions);
			_children[1][1][1]->potentialCollisions(collisions);
		}
	}
	else
	{
		//std::cout << "Child " << ChildNumber << " has " << _colliders.size() << " colliders\n";
		//Add all pairs (colliderA, colliderB) from _colliders
		int size = colliders.size();
		for(int i = 0; i < size; i++)
		{
			while (!OctreeManager::barrier[0].try_lock());

			GLint colliderA = (colliders)[i];
			for(int j = i + 1; j < size; j++)
			{
				GLint colliderB = (colliders)[j];
				ColliderPair cp;
				cp.colliderA = colliderA;
				cp.colliderB = colliderB;
				collisions.push_back(cp);
			}

			OctreeManager::barrier[0].unlock();
		}
	}
}

//Meant to be called by parent to pair one of its colliders with colliders in this
void Octree::collideWithParent(std::vector<ColliderPair>& collisions, GLint collider)
{
	while (!OctreeManager::barrier[0].try_lock());

	int size = colliders.size();
	for(int i = 0; i < size; i++)
	{
		ColliderPair cp;
		cp.colliderA = collider;
		cp.colliderB = (colliders)[i];
		collisions.push_back(cp);
	}

	OctreeManager::barrier[0].unlock();
}

void Octree::potentialWallCollisions(std::vector<ColliderWallPair> &collisions)
{
	potentialWallCollisions(collisions, WALL_LEFT, 'x', 0);
	potentialWallCollisions(collisions, WALL_RIGHT, 'x', 1);
	potentialWallCollisions(collisions, WALL_BOTTOM, 'z', 0);
	potentialWallCollisions(collisions, WALL_TOP, 'z', 1);
	potentialWallCollisions(collisions, WALL_FAR, 'y', 1);
	potentialWallCollisions(collisions, WALL_NEAR, 'y', 0);
}

//Interface with collider to evaluate and resolve collisions
void Octree::update(GLfloat time)
{
	std::vector<ColliderWallPair> wallCollisions;
	//wallCollisions.reserve(4);
	this->potentialWallCollisions(wallCollisions);
	std::vector<ColliderPair> collisions;
	//collisions.reserve(4);
	this->potentialCollisions(collisions);
	std::map<int, int>& movement = colliderObj->update(collisions, wallCollisions, time);
	//update the tree
	moveColliders(movement);
}

////////////////Internal Funcitons////////////////////////

//Adds a ball to or removes on from the children of this
void Octree::fileCollider(GLint collider, bool addColl, bool storeInThis)
{
	//Figure out in which singular child the collider belongs
	int child[] = {0, 0, 0};
	for(int i = 0; i < 3; i++)
	{
		//If coll is in right child, move right/up/out
		if(getColliderPos(collider)[i] + getColliderRad(collider) > center[i])
			child[i]++;
		//If coll is in left child, move left/down/in
		if(getColliderPos(collider)[i] - getColliderRad(collider) < center[i])
			child[i]--;
		//If coll is in more than 1, add to parent
		if(child[i] == 0)
		{
			if(storeInThis)
				this->add(collider, true);
			return;
		}
	}
	//If coll is in a child, find child and add
	if(hasChildren)
	{
		for(int i = 0; i < 3; i++)
		{
			if(child[i] == -1)
				child[i] = 0;
		}
		_children[child[0]][child[1]][child[2]]->add(collider, false);
	}
	else if(storeInThis)
	{
		this->add(collider, true);
	}
}

//Creates children of this, and moves the balls in this to the children
void Octree::haveChildren()
{
	for(int i = 0; i < 2; i++)
	{
		float minX, maxX;
		if(i == 0)
		{
			minX = corner1[0];
			maxX = center[0];
		}
		else
		{
			minX = center[0];
			maxX = corner2[0];
		}

		for(int j = 0; j < 2; j++)
		{
			float minY, maxY;
			if(j == 0)
			{
				minY = corner1[1];
				maxY = center[1];
			}
			else
			{
				minY = center[1];
				maxY = corner2[1];
			}

			for(int k = 0; k < 2; k++)
			{
				float minZ, maxZ;
				if(k == 0)
				{
					minZ = corner1[2];
					maxZ = center[2];
				}
				else
				{
					minZ = center[2];
					maxZ = corner2[2];
				}

				_children[i][j][k] = OctreeManager::initChild(glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ), depth, colliderObj, id, walls, center, i * 4 + j * 2 + k + 1);
			}
		}
	}
	hasChildren = true;
	//Remove all colliders from "_colliders" and add them to the new children or self
	std::vector<GLint> tmp = colliders;
	colliders.clear();
	addAll(tmp);
}

//Adds all the colliders in this or one of its desendants to the specified set
void Octree::collectBalls(std::vector<GLint> &colliders)
{
	if(hasChildren)
	{
		if (depth == -1)
		{
			Octree *(*children)[2][2] = _children;
			parallel_invoke(
				[&colliders, &children] { children[0][0][1]->collectBalls(colliders); },
				[&colliders, &children] { children[0][0][0]->collectBalls(colliders); },
				[&colliders, &children] { children[0][1][1]->collectBalls(colliders); },
				[&colliders, &children] { children[0][1][0]->collectBalls(colliders); },
				[&colliders, &children] { children[1][0][1]->collectBalls(colliders); },
				[&colliders, &children] { children[1][0][0]->collectBalls(colliders); },
				[&colliders, &children] { children[1][1][1]->collectBalls(colliders); },
				[&colliders, &children] { children[1][1][0]->collectBalls(colliders); }
			);
		}
		else
		{
			_children[0][0][0]->collectBalls(colliders);
			_children[0][0][1]->collectBalls(colliders);
			_children[0][1][0]->collectBalls(colliders);
			_children[0][1][1]->collectBalls(colliders);
			_children[1][0][0]->collectBalls(colliders);
			_children[1][0][1]->collectBalls(colliders);
			_children[1][1][0]->collectBalls(colliders);
			_children[1][1][1]->collectBalls(colliders);
		}
	}
	else
	{
		while (!OctreeManager::barrier[1].try_lock());

		unsigned int size = colliders.size();
		for (unsigned int i = 0; i < size; i++)
		{
			GLint collider = colliders[i];
			colliders.push_back(collider);
		}

		OctreeManager::barrier[1].unlock();
	}
}

//Destroys the children of this, and moves all colliders in its descendants to the "_colliders" set
void Octree::destroyChildren()
{
	//Move all the balls in descendants of this to the "_colliders" set
	collectBalls(colliders);
	OctreeManager::deactivateChildren(id, depth);

	hasChildren = false;
}

void Octree::potentialWallCollisions(std::vector<ColliderWallPair> &colliderWallPairs, Wall w, char coord, int dir)
{
	//Check to see if this borders the wall in question
	if(walls[w])
	{
		while (!OctreeManager::barrier[2].try_lock());

		int size = colliders.size();
		for(int i = 0; i < size; i++)
		{
			GLint collider = (colliders)[i];
			ColliderWallPair cwp;
			cwp.collider = collider;
			cwp.wall = w;
			colliderWallPairs.push_back(cwp);
		}

		OctreeManager::barrier[2].unlock();
		//Check to see if this has children to get colliders from
		if(hasChildren)
		{
			_children[0][0][0]->potentialWallCollisions(colliderWallPairs, w, coord, dir);
			_children[0][0][1]->potentialWallCollisions(colliderWallPairs, w, coord, dir);
			_children[0][1][0]->potentialWallCollisions(colliderWallPairs, w, coord, dir);
			_children[0][1][1]->potentialWallCollisions(colliderWallPairs, w, coord, dir);
			_children[1][0][0]->potentialWallCollisions(colliderWallPairs, w, coord, dir);
			_children[1][0][1]->potentialWallCollisions(colliderWallPairs, w, coord, dir);
			_children[1][1][0]->potentialWallCollisions(colliderWallPairs, w, coord, dir);
			_children[1][1][1]->potentialWallCollisions(colliderWallPairs, w, coord, dir);
		}
	}
	//Else, this and its children are not important
}
	
void Octree::moveColliders(std::map<GLint, GLint> &colliders)
{
	removeAll(colliders);
	addAll(colliders);
}

glm::vec3 Octree::wallDirection(Wall wall)
{
	switch(wall)
	{
	case WALL_LEFT:
		return glm::vec3(-1, 0, 0);
	case WALL_RIGHT:
		return glm::vec3(1, 0, 0);
	case WALL_FAR:
		return glm::vec3(0, 1, 0);
	case WALL_NEAR:
		return glm::vec3(0, -1, 0);
	case WALL_TOP:
		return glm::vec3(0, 0, 1);
	case WALL_BOTTOM:
		return glm::vec3(0, 0, -1);
	default:
		return glm::vec3(0, 0, 0);
	}
}

bool Octree::checkChild(GLint collider)
{
	//Check to see if the collider will actually fit in the new division
	return colliderObj->host(collider).radius * 2 < corner2.x - center.x;
}

void Octree::addAll(std::map<GLint, GLint>& colliders)
{
	for(std::map<GLint, GLint>::iterator it = colliders.begin(); it != colliders.end(); it++)
	{
		GLint collider = it->first;
		fileCollider(collider, true, true);
	}
}

void Octree::addAll(std::vector<GLint>& colliders)
{
	for(int i = 0; i < colliders.size(); i++)
	{
		fileCollider(colliders[i], true, true);
	}
}

void Octree::removeAll(std::map<GLint, GLint>& colliders)
{
	//Remove from scope any colliders passed in arg
	while(OctreeManager::barrier[1].try_lock());

	std::vector<GLint> survivors;
	survivors.reserve(4);
	int size = this->colliders.size();
	for(int i = 0; i < size; i++)
	{
		if(colliders[this->colliders[i]] == -1)
			survivors.push_back(this->colliders[i]);
	}
	(this->colliders) = survivors;

	OctreeManager::barrier[1].unlock();
	//this->colliders->clear();
	//this->colliders 
	//recurse into children(if there are any)
	if(hasChildren)
	{

		_children[0][0][0]->removeAll(colliders);
		_children[0][1][0]->removeAll(colliders);
		_children[0][0][1]->removeAll(colliders);
		_children[0][1][1]->removeAll(colliders);
		_children[1][0][0]->removeAll(colliders);
		_children[1][1][0]->removeAll(colliders);
		_children[1][0][1]->removeAll(colliders);
		_children[1][1][1]->removeAll(colliders);
	}
}

//Helper function for determining bordering walls in constructor
void Octree::checkWalls(std::map<Wall, bool>& parentWalls, std::map<Wall, bool>& _walls, glm::vec3& parentCenter, glm::vec3& corner1, glm::vec3& corner2, bool root)
{
	//Check to see if this is root
	if(root)
	{
		//This is root
		_walls[WALL_LEFT] = true;
		_walls[WALL_RIGHT] = true;
		_walls[WALL_FAR] = true;
		_walls[WALL_NEAR] = true;
		_walls[WALL_TOP] = true;
		_walls[WALL_BOTTOM] = true;
	}
	//Otherwise, check for walls
	else
	{
		_walls[WALL_LEFT] = false;
		_walls[WALL_RIGHT] = false;
		_walls[WALL_FAR] = false;
		_walls[WALL_NEAR] = false;
		_walls[WALL_TOP] = false;
		_walls[WALL_BOTTOM] = false;
		//Does parent border left wall?
		if(parentWalls[WALL_LEFT])
		{
			//True->if corner1 is left of parent's center
			if(corner1.x < parentCenter.x)
			{
				//This octree borders left wall
				_walls[WALL_LEFT] = true;
			}
		}
		//Does parent border Right wall?
		if(parentWalls[WALL_RIGHT])
		{
			//True->if corner2 is Right of parent's center
			if(corner2.x > parentCenter.x)
			{
				//This octree borders Right wall
				_walls[WALL_RIGHT] = true;
			}
		}
		//Does parent border Far wall?
		if(parentWalls[WALL_FAR])
		{
			//True->if corner2 is Behind parent's center
			if(corner2.y > parentCenter.y)
			{
				//This octree borders Far wall
				_walls[WALL_FAR] = true;
			}
		}
		//Does parent border Near wall?
		if(parentWalls[WALL_NEAR])
		{
			//True->if corner1 is in front of parent's center
			if(corner1.y < parentCenter.y)
			{
				//This octree borders Near wall
				_walls[WALL_NEAR] = true;
			}
		}
		//Does parent border Top wall?
		if(parentWalls[WALL_TOP])
		{
			//True->if corner2 is above parent's center
			if(corner2.z > parentCenter.z)
			{
				//This octree borders Top wall
				_walls[WALL_TOP] = true;
			}
		}
		//Does parent border bottom wall?
		if(parentWalls[WALL_BOTTOM])
		{
			//True->if corner1 is in below parent's center
			if(corner1.z < parentCenter.z)
			{
				//This octree borders bottom wall
				_walls[WALL_BOTTOM] = true;
			}
		}
	}
}

glm::vec3 Octree::getColliderPos(GLint collider)
{
	return colliderObj->host(collider).host->transform(colliderObj->host(collider).hostInstance).position();
}

GLfloat Octree::getColliderRad(GLint collider)
{
	return colliderObj->host(collider).radius;
}