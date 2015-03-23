#include "utils.h";
#include "icosphere.h"
#include "teapot.h"
#include "sphereCollider.h"
//#include "octree.h"
#include "octreeManager.h"
#include <fstream>
#pragma comment(linker, "/nodefaultlib:libc.lib")
#pragma comment(linker, "/nodefaultlib:libcd.lib")

//window
GLFWwindow* window;

GLuint program;

//Texture
GLuint tex;
GLfloat pixels[] = { 
	0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f,	1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,	0.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,
	1.0f, 0.5f, 0.5f,	0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f
};
GLfloat collideColor[] = {1.0, 0.0f, 0.0f};

float prevTime = 0;

Teapot* pots;
Icosphere* spheres;
const int NUM_TEAPOTS = 0;
const int NUM_SPHERES = 100;
const float WORLD = 3.0f;
const GLfloat RADIUS = 0.05f;

SphereCollider* collider;

Octree* tree;

////////
// Custom allocation routines
////////
long howMuchMemory = 0;
void* operator new(size_t size)
{
//	std::cout << "New called with size: " << size << std::endl;
	//howMuchMemory += size;
	return malloc(size * sizeof(char));
}


void operator delete(void* mem)
{
	//std::cout << "delete called for mem of size: " << _msize(mem) << std::endl;

	free(mem);
}

void* operator new[](size_t size)
{
	//std::cout << "New[] called with size: " << size << std::endl;
	//howMuchMemory += size;
	return malloc(size * sizeof(char));
}

void operator delete[](void* mem) {
	//std::cout << "delete[] called for mem of size: " << _msize(mem) << std::endl;

	free(mem);
}

// altered version of crt_heapwalk.c
// This program "walks" the heap, starting
// at the beginning (_pentry = NULL). It prints out each
// heap entry's use, location, and size. It also prints
// out information about the overall state of the heap as
// soon as _heapwalk returns a value other than _HEAPOK
// or if the loop has iterated 300 times.

void heapdump(void)
{
	_HEAPINFO hinfo;
	int heapstatus;
	int numLoops;
	hinfo._pentry = NULL;
	numLoops = 0;
	std::ofstream output;
	output.open("heap.txt", std::ios::out | std::ios::app);
	output << "Starting heap dump " << std::endl;

	while ((heapstatus = _heapwalk(&hinfo)) == _HEAPOK &&
		numLoops < 500)
	{
		output << (hinfo._useflag == _USEDENTRY ? "USED" : "FREE") << " block at " << hinfo._pentry << " of size " << hinfo._size << std::endl;
		numLoops++;
	}

	switch (heapstatus)
	{
	case _HEAPEMPTY:
		output << "OK - empty heap" << std::endl;
		break;
	case _HEAPEND:
		output << "OK - end of heap" << std::endl;
		break;
	case _HEAPBADPTR:
		output << "ERROR - bad pointer to heap " << std::endl;
		break;
	case _HEAPBADBEGIN:
		output << "ERROR - bad start of heap" << std::endl;
		break;
	case _HEAPBADNODE:
		output << "ERROR - bad node in heap" << std::endl;
		break;
	}

	output << "Ending heap dump" << std::endl;
	output.close();

}

void setupSpheres()
{
	//Apply random rotations and velocities to each instance
	/*
	srand(time(NULL));
	for(int i = 0; i < NUM_TEAPOTS; i++)
	{
		pots->transform(i).rotation = glm::vec4(glm::ballRand(1.0f), glm::circularRand(90.0f).x);
		pots->transform(i).velocity = glm::ballRand(1.0f);
		pots->transform(i).scale = glm::scale(glm::vec3(RADIUS));
	}
	*/
	for(int i = 0; i < NUM_SPHERES; i++)
	{
		spheres->transform(i).rotation = glm::vec4(glm::ballRand(1.0f), glm::circularRand(90.0f).x);
		spheres->transform(i).velocity = glm::ballRand(1.0f);
		spheres->transform(i).scale = glm::scale(glm::vec3(RADIUS));
		spheres->transform(i).velocity.z = -.1;
	}
	//spheres->transform(0).velocity.y = -1.5;
	/*
	spheres->transform(0).translate(glm::translate(glm::vec3(-0.7f, -0.7f, 0.5f)));
	spheres->transform(1).translate(glm::translate(glm::vec3(0.0f, -0.7f, 0.5f)));
	spheres->transform(2).translate(glm::translate(glm::vec3(0.7f, -0.7f, 0.5f)));
	spheres->transform(3).translate(glm::translate(glm::vec3(-0.7f, .0f, 0.5f)));
	spheres->transform(4).translate(glm::translate(glm::vec3(0.0f, .0f, 0.5f)));
	spheres->transform(5).translate(glm::translate(glm::vec3(0.7f, .0f, 0.5f)));
	spheres->transform(6).translate(glm::translate(glm::vec3(-0.7f, 0.7f, 0.5f)));
	spheres->transform(7).translate(glm::translate(glm::vec3(0.0f, 0.7f, 0.5f)));
	spheres->transform(8).translate(glm::translate(glm::vec3(0.7f, 0.7f, 0.5f)));
	*/
}


void initiateProgram()
{
	pots = new Teapot(NUM_TEAPOTS);
	spheres = new Icosphere(NUM_SPHERES);
	collider = new SphereCollider(WORLD);
	//tree = new Octree(glm::vec3(-WORLD), glm::vec3(WORLD), 1, collider, 0, std::map<Wall, bool>(), glm::vec3(), true);
	OctreeManager::createOctree(glm::vec3(-WORLD), glm::vec3(WORLD), collider);
	//SphereCollider must be intialized with each object type
	collider->init(NUM_TEAPOTS, pots, RADIUS * 2.0f);
	collider->init(NUM_SPHERES, spheres, RADIUS);
	spheres->init("fshader.glsl", "vshader.glsl", pixels, 4, 4, collideColor, 1, 1);
	pots->init("fshader.glsl", "vshader.glsl", pixels, 4, 4, collideColor, 1, 1);
	//tree->init();
	OctreeManager::init();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	setupSpheres();
}

void update()
{
	float time = glfwGetTime();
	//All collisions are evaluated and resloved before updating positons
	float dt = time - prevTime;
	//tree->update(time - prevTime);
	OctreeManager::updateTree(dt);
	pots->update(dt);
	spheres->update(dt);
	prevTime = time;
}

void display()
{
	glClearColor(0.8f, 0.8f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for(int i = 0; i < NUM_TEAPOTS; i++)
	{
		pots->display(i);
	}
	for(int i = 0; i < NUM_SPHERES; i++)
	{
		spheres->display(i);
	}
}

void step()
{
	update();
	display();
}

int main()
{
	glfwInit();

	//Create window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL); // Windowed
	
	//Activate window
	glfwMakeContextCurrent(window);

	//Hook up to gpu
	glewExperimental = GL_TRUE;
	glewInit();

	//Baseline heap
	//heapdump();

	//Important stuff
	initiateProgram();

	//Post-initiation
	//heapdump();

	//program loop
	while(!glfwWindowShouldClose(window))
	{
		step();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	delete pots;
	delete spheres;
	delete collider;
	glfwTerminate();

	//heapdump();
}