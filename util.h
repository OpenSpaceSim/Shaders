#ifndef __util_h
#define __util_h

#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/freeglut.h>
#endif

#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

// assimp include files. These three are usually needed.
#include "assimp.h"
#include "aiPostProcess.h"
#include "aiScene.h"
using namespace std;

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)

//load Bitmap file
// Struct of bitmap file.
struct BitMapFile {
	int sizeX;
	int sizeY;
	unsigned short bitDepth;
	unsigned char *data;
};

typedef struct aiVector3D vertex;

extern const struct aiScene* scene;
extern struct aiVector3D scene_min, scene_max, scene_center;

extern GLuint vertexCount;
extern vertex* vertices;
BitMapFile *getBMPData(string filename);
void checkError();
void calculateSurfaceNormal(const GLfloat* polygon, GLint count, GLfloat* normal);
void calculateModelNormals(vertex* normals, GLint polygonSize);
void loadModel();
void BuildPerspProjMat(float *m, float fov, float aspect, float znear, float zfar);

void get_bounding_box_for_node (const struct aiNode* nd, 
	struct aiVector3D* min, 
	struct aiVector3D* max, 
	struct aiMatrix4x4* trafo
);

void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max);

int loadasset (const char* path);
bool LoadTextureFromBitmap(const char *file, GLuint texture);
#endif
