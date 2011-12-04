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
using namespace std;

//load Bitmap file
// Struct of bitmap file.
struct BitMapFile {
   int sizeX;
   int sizeY;
   unsigned char *data;
};

struct vertex {
    float x;
    float y;
    float z;
};
extern GLuint vertexCount;
extern vertex* vertices;
BitMapFile *getBMPData(string filename);
void checkError();
void calculateSurfaceNormal(const GLfloat* polygon, GLint count, GLfloat* normal);
void calculateModelNormals(vertex* normals, GLint polygonSize);
void loadModel();
void BuildPerspProjMat(float *m, float fov, float aspect, float znear, float zfar);
#endif
