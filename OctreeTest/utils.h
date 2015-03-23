#ifndef UTILS_H
#define UTILS_H

//Include GLEW  
#include <GL/glew.h>
  
//Include GLFW  
//#include <GL/glfw3.h>
#include <GLFW\glfw3.h>
  
//Include the standard C++ headers  
#include <stdio.h>  
#include <stdlib.h>  
#include <iostream>
#include <cmath>
#include <vector>
#include <map>
#include <List>
  
// Include GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//defines
#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))
#define DEGREES_TO_RADIANS 3.1415926535897932384626833/180
#define GLEW_STATIC

#endif