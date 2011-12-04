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

#include "shader.h"

using namespace std;

//load Bitmap file
// Struct of bitmap file.
struct BitMapFile
{
   int sizeX;
   int sizeY;
   unsigned char *data;
};

// Routine to read a bitmap file.
// Works only for uncompressed bmp files of 24-bit color.
BitMapFile *getBMPData(string filename)
{
   BitMapFile *bmp = new BitMapFile;
   unsigned int size, offset, headerSize;
  
   // Read input file name.
   ifstream infile(filename.c_str(), ios::binary);
 
   // Get the starting point of the image data.
   infile.seekg(10);
   infile.read((char *) &offset, 4); 
   
   // Get the header size of the bitmap.
   infile.read((char *) &headerSize,4);

   // Get width and height values in the bitmap header.
   infile.seekg(18);
   infile.read( (char *) &bmp->sizeX, 4);
   infile.read( (char *) &bmp->sizeY, 4);

   // Allocate buffer for the image.
   size = bmp->sizeX * bmp->sizeY * 24;
   bmp->data = new unsigned char[size];

   // Read bitmap data.
   infile.seekg(offset);
   infile.read((char *) bmp->data , size);
   
   // Reverse color from bgr to rgb.
   int temp;
   for (int i = 0; i < size; i += 3)
   { 
      temp = bmp->data[i];
	  bmp->data[i] = bmp->data[i+2];
	  bmp->data[i+2] = temp;
   }

   return bmp;
}


struct vertex
{
    float x;
    float y;
    float z;
};

GLuint vertexCount;
vertex* vertices;
vertex* normals;
GLfloat* texCoords;
Shader* shader;
GLuint vertexArrays[2];
GLuint vertexBuffers[2];
GLuint textures[2];

int wireframe = 0;
int show_normals = 0;
int lighting = 1;
int culling = 1;
float alpha = 1.0f;

inline void checkError()
{
    GLenum err = glGetError();
    if(err != GL_NO_ERROR)
    {
    	cout << gluErrorString(err);
    	if(err == GL_INVALID_ENUM)
    	{ 
    		cout <<" (GL_INVALID_ENUM)" << endl;
    		return;
	}
    	if(err == GL_INVALID_VALUE)
    	{
    		cout << " (GL_INVALID_VALUE)"<<endl;
    		return;
	}
	if(err == GL_INVALID_OPERATION)
	{
		cout << " (GL_INVALID_OPERATION)"<<endl;
		return;
	}
	cout << " (" << err << ")" << endl;
    }
}

void loadModel()
{
    ifstream file("ncc1701.data");
    if(!file.is_open())
    {
        cout << "unable to open file \n";
        return;
    }
    vector<vertex> vertvec;
    while(!file.eof())
    {
        vertex point;
        file >> point.x;
        file >> point.y;
        file >> point.z;
        vertvec.push_back(point);
    }
    vertices = new vertex[vertvec.size()];
    for(int i=0;i<vertvec.size();i++)
    {
    	vertices[i].x = vertvec[i].x;
    	vertices[i].y = vertvec[i].y;
    	vertices[i].z = vertvec[i].z;
    }
    vertexCount = vertvec.size();
}
//calculates average surface normal for arbitrary polygon
void calculateSurfaceNormal(const GLfloat* polygon, GLint count, GLfloat* normal)
{
        normal[0] = 0.0f;
        normal[1] = 0.0f;
        normal[2] = 0.0f;
        for(int i=0;i<(count*3);i+=3)
        {
                const GLfloat* current = polygon+(i);
                const GLfloat* next = polygon+(((i+3)%(count*3)));
                GLfloat dx = current[0] - next[0];
                GLfloat dy = current[1] - next[1];
                GLfloat dz = current[2] - next[2];
                GLfloat sx = current[0] + next[0];
                GLfloat sy = current[1] + next[1];
                GLfloat sz = current[2] + next[2];
                normal[0] += dy * sz;
                normal[1] += dz * sx;
                normal[2] += dx * sy;
        }
        //normalize result
        GLfloat avg = sqrt(normal[0]*normal[0]+normal[1]*normal[1]+normal[2]*normal[2]);
        normal[0] /= avg;
        normal[1] /= avg;
        normal[2] /= avg;
}

//calculate normals for each polygon in the model
void calculateModelNormals(vertex* normals, GLint polygonSize)
{
        GLfloat normal[3];
        GLfloat* polygon = (GLfloat*)malloc(3 * sizeof(GLfloat*) * polygonSize);
        for(int i=0; (i+polygonSize)<=vertexCount; i+= polygonSize)
        {
        	//cout << "i=" << i << endl;
        	int k=0;
                for(int j=0;j<3*polygonSize;j+=3)
                {
                	if(!((i+j+2)<(vertexCount*3)))
                	{
                		cout << "ERROR:vertices not even multiple of polygon size\n";
                		return;
                	}
                        polygon[j] = vertices[i+k].x;
                        polygon[j+1] = vertices[i+k].y;
                        polygon[j+2] = vertices[i+k].z;
                        k++;
                }
                calculateSurfaceNormal(polygon,polygonSize,normal);
                for(int j=0;j<polygonSize;j++)
                {
                        normals[i+j].x = normal[0];
                        normals[i+j].y = normal[1];
                        normals[i+j].z = normal[2];
                }
        }
}

/* GLUT callback Handlers */

GLfloat viewMatrix[16];

//lighting and material information
const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.3f, 1.0f };
const GLfloat light_diffuse[]  = { 0.8f, 0.6f, 0.4f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat light_position[] = { 2.0f, 0.5f, 0.0f, 1.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

void BuildPerspProjMat(float *m, float fov, float aspect,
float znear, float zfar)
{
  float xymax = znear * tan(fov * (3.14159/360.0));
  float ymin = -xymax;
  float xmin = -xymax;

  float width = xymax - xmin;
  float height = xymax - ymin;

  float depth = zfar - znear;
  float q = -(zfar + znear) / depth;
  float qn = -2 * (zfar * znear) / depth;

  float w = 2 * znear / width;
  w = w / aspect;
  float h = 2 * znear / height;
  

  m[0]  = w;
  m[1]  = 0;
  m[2]  = 0;
  m[3]  = 0;

  m[4]  = 0;
  m[5]  = h;
  m[6]  = 0;
  m[7]  = 0;

  m[8]  = 0;
  m[9]  = 0;
  m[10] = q;
  m[11] = -1;

  m[12] = 0;
  m[13] = 0;
  m[14] = qn;
  m[15] = 0;
}

static void resize(int width, int height)
{
    glViewport(0, 0, width, height);
    const float ar = (float) width / (float) height;
    BuildPerspProjMat(viewMatrix, 90.0, ar, 0.1, 100);
}

static void display(void)
{
    const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    const double a = t*0.5;
    
    //rotate around y axis
     GLfloat modelMatrix[] = {
    	cos(a),0.0f,sin(a),0.0f,
    	0.0f,1.0f,0.0f,0.0f,
    	-sin(a),0.0f,cos(a),0.0f,
    	0.0f,0.0f,0.0f,1.0f};
    	
	//rotate around x axis
    /*GLfloat modelMatrix[] = {
    	1.0f,0.0f,0.0f,0.0f,
    	0.0f,cos(a),-sin(a),0.0f,
    	0.0f,sin(a),cos(a),0.0f,
    	0.0f,0.0f,0.0f,1.0f};*/
    /*GLfloat modelMatrix[] = {
    	1.0f,0.0f,0.0f,0.0f,
    	0.0f,1.0f,0.0f,0.0f,
    	0.0f,0.0f,1.0f,0.0f,
    	0.0f,0.0f,0.0f,1.0f};*/

    GLint modelLoc = glGetUniformLocation(shader->id(),"modelMatrix");
    GLint viewLoc = glGetUniformLocation(shader->id(),"viewMatrix");
    GLint scalarLoc = glGetUniformLocation(shader->id(),"scalar");
    GLint lightLoc = glGetUniformLocation(shader->id(),"lightPosition");
    GLint lightAmb = glGetUniformLocation(shader->id(),"lightAmbient");
    GLint lightDiff = glGetUniformLocation(shader->id(),"lightDiffuse");
    GLint lightSpec = glGetUniformLocation(shader->id(),"lightSpecular");
    GLint colorTex = glGetUniformLocation(shader->id(),"colorTex");
    shader->bind();
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix);
    glUniform1f(scalarLoc,5.0f);
    glUniform4fv(lightLoc,1,light_position);
    glUniform4fv(lightAmb,1,light_ambient);
    glUniform4fv(lightDiff,1,light_diffuse);
    glUniform4fv(lightSpec,1,light_specular);
    glUniform1i(colorTex,0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,textures[0]);

    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    
    
    
    
    glBindVertexArray(vertexArrays[0]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);
    
    glBindVertexArray(0);
    
    shader->unbind();
    
    glutSwapBuffers();
}


static void key(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27 :
        case 'q':
            exit(0);
            break;
	case 'c':
		culling = !culling;
		if(culling)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
		break;
    }

    glutPostRedisplay();
}
double last = 0.0;
static void idle(void)
{
    const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    if((t - last) > (1.0/60.0))
    {
    	last = t;
	glutPostRedisplay();
    }
}

int main(int argc, char *argv[])
{
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    cout << "GLUT version " << glutGet(GLUT_VERSION) << endl;
    
    glutInitContextVersion(3, 2);
    //glutInitContextFlags (GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
    checkError();
    glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
    checkError();
    glutInitWindowPosition(10,10);
    checkError();
    glutInitWindowSize(640,480);
    checkError();
    
    glutCreateWindow("Open Space Sim");
    
    checkError();
    cout << "GLUT setup complete\n";
    
    glewExperimental=GL_TRUE;
	GLenum err=glewInit();
	if(err!=GLEW_OK)
	{
		//Problem: glewInit failed, something is seriously wrong.
		cout<<"glewInit failed, aborting."<<endl;
		exit(1);
	}
	
	checkError();
    cout << "GLEW setup complete\n";
    
    cout<<"OpenGL version = "<<glGetString(GL_VERSION)<<endl;
    
    int glVersion[2] = {0, 0};
    glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
    std::cout << "Using OpenGL: " << glVersion[0] << "." << glVersion[1] << std::endl;
    
    int textureUnits;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,&textureUnits);
    std::cout << "Maximum image texture units: " << textureUnits << endl;

    checkError();
    
    BuildPerspProjMat(viewMatrix, 90.0, 4.0/3.0, 0.1, 100);

    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutIdleFunc(idle);
    
    checkError();
    cout << "Setup GLUT callbacks" << endl;

    glClearColor(0,0,0,0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    checkError();
    cout << "GL setup complete" << endl;

    loadModel();
    
    normals = new vertex[vertexCount];
    calculateModelNormals(normals,4);
    
    vertex* newNormals = new vertex[vertexCount];
    
    for(int i=0;i<vertexCount;i++)
    {
    	float count = 1.0f;
    	newNormals[i].x = normals[i].x;
    	newNormals[i].y = normals[i].y;
    	newNormals[i].z = normals[i].z;
    	for(int j=0;j<vertexCount;j++)
    	{
    		if(vertices[i].x == vertices[j].x && vertices[i].y == vertices[j].y && vertices[i].z == vertices[j].z)
    		{
    			newNormals[i].x += normals[j].x;
    			newNormals[i].y += normals[j].y;
    			newNormals[i].z += normals[j].z;
    			count++;
    		}
	}
	newNormals[i].x /= count;
	newNormals[i].y /= count;
	newNormals[i].z /= count;
    }
    
    
    texCoords = new GLfloat[vertexCount*2];
    for(int i=0;i<vertexCount*2;i+=2)
    {
    	switch(i%8)
    	{
    		case 0:
    			texCoords[i] = 1.0f;
    			texCoords[i+1] = 1.0f;
    			break;
		case 2:
    			texCoords[i] = 0.0f;
    			texCoords[i+1] = 1.0f;
    			break;
		case 4:
    			texCoords[i] = 1.0f;
    			texCoords[i+1] = 0.0f;
    			break;
		case 6:
			texCoords[i] = 0.0f;
    			texCoords[i+1] = 0.0f;
    			break;
	}
    }
    checkError();
    glGenVertexArrays(1, vertexArrays);
    checkError();
    glBindVertexArray(vertexArrays[0]);
    checkError();
    glGenBuffers(3, vertexBuffers);
    checkError();
    glBindBuffer(GL_ARRAY_BUFFER,vertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexCount*sizeof(vertex), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER,vertexBuffers[1]);
    checkError();
    glBufferData(GL_ARRAY_BUFFER, vertexCount*sizeof(GLfloat)*2, texCoords, GL_STATIC_DRAW);
    checkError();
    glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    checkError();
    glBindBuffer(GL_ARRAY_BUFFER,vertexBuffers[2]);
    glBufferData(GL_ARRAY_BUFFER, vertexCount*sizeof(vertex), newNormals, GL_STATIC_DRAW);
    checkError();
    glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    checkError();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    
    checkError();
    
    shader = new Shader("shader.vert","shader.frag");
    glBindAttribLocation(shader->id(), vertexBuffers[0], "in_Position"); // Bind a constant attribute location for positions of vertices
	glBindAttribLocation(shader->id(), vertexBuffers[1], "in_TexCoords"); // Bind another constant attribute location, this time for color
	glBindAttribLocation(shader->id(), vertexBuffers[2], "in_Normal");
    
    checkError();
    cout << "allocating texture" <<endl;
    glGenTextures(1,textures);
    checkError();
    cout << "loading image from file" << endl;
    BitMapFile* image = getBMPData("brick.bmp");
    cout << "binding texture" << endl;
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    checkError();
    cout << "setting texture parameters" << endl;
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  checkError();
  cout << "loading texture data" << endl;
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB, image->sizeX, image->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
  checkError();
  cout << "generating mipmaps" << endl;
  glGenerateMipmap(GL_TEXTURE_2D); //Generate mipmaps now!!!
//  delete image; //free our copy of the image
  
  checkError();

    glutMainLoop();

    return EXIT_SUCCESS;
}
