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
#include <stack>

// assimp include files. These three are usually needed.
#include "assimp.h"
#include "aiPostProcess.h"
#include "aiScene.h"

#include "shader.h"
#include "util.h"

using namespace std;

ostream &operator<<(ostream &ostr, const aiMatrix4x4 &o) {
	ostr << "[" <<o.a1 << ", " <<o.a2 << ", " <<o.a3 << ", " <<o.a4 << "\n";
	ostr << " " <<o.b1 << ", " <<o.b2 << ", " <<o.b3 << ", " <<o.b4 << "\n";
	ostr << " " <<o.c1 << ", " <<o.c2 << ", " <<o.c3 << ", " <<o.c4 << "\n";
	ostr << " " <<o.d1 << ", " <<o.d2 << ", " <<o.d3 << ", " <<o.d4 << "]\n";
	return ostr;
}
GLfloat *operator*(aiMatrix4x4 &o) {
	return (GLfloat*)&o;
}
const GLfloat *operator*(const aiMatrix4x4 &o) {
	return (const GLfloat*)&o;
}
GLfloat *operator*(aiColor4D &o) {
	return (GLfloat*)&o;
}
const GLfloat *operator*(const aiColor4D &o) {
	return (const GLfloat*)&o;
}

GLuint vertexCount;
vertex* vertices;
vertex* normals;
vertex* tangents;
vertex* texCoords;
Shader* shader;
GLuint vertexArrays[2];
GLuint vertexBuffers[4];
GLuint textures[2];

aiMatrix4x4 modelMatrix;

const struct aiScene* scene = NULL;
struct aiVector3D scene_min, scene_max, scene_center;

int wireframe = 0;
int show_normals = 0;
int lighting = 1;
int culling = 1;
float alpha = 1.0f;

typedef struct aiVector3D vector;

/* GLUT callback Handlers */

GLfloat viewMatrix[16];

//lighting and material information
const aiColor4D light_ambient(0.1f, 0.1f, 0.1f, 1.0f);
const aiColor4D light_diffuse(1.0f, 1.0f, 1.0f, 1.0f);
const aiColor4D light_specular(1.0f, 1.0f, 1.0f, 1.0f);

const GLfloat light_position[] = { 0.0f, 0.0f, 1.0f, 1.0f };


const GLfloat mat_ambient[]	= { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]	= { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

static void resize(int width, int height) {
	glViewport(0, 0, width, height);
	const float ar = (float) width / (float) height;
	BuildPerspProjMat(viewMatrix, 90.0, ar, 0.1, 100);
}
double off = -0.5;
double lastT = 0.0;
double inc = 0.002;
float trans[] = {0.0f, -0.25f, 0.8f, 1.0f};
//float trans[] = {0.0f, 0.0, 0.0, 1.0f};
static void display(void) {
	const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	const double a = t*0.1;
	
	off += inc;
	if(abs(off)>0.5)
	        inc = -inc;
	trans[0] = off;
	GLfloat shiftedLight[4];
	for(int i=0;i<4;i++)
	{
	        shiftedLight[i] = light_position[i];
        }
        shiftedLight[0] += off;
	//rotate around y axis
	aiMatrix4x4 rotation, tmp;
	//aiMatrix4x4::RotationY(a,rotation);
	//aiMatrix4x4::Translation(aiVector3D(0.0f,0.0f,0.0f),rotation);
	//aiMatrix4x4::RotationY(-3.1415926535/2,tmp);
	//rotation *=tmp;
	rotation *= modelMatrix;
	
	shader->bind();
	shader->uniformMatrix4fv("modelMatrix",*rotation);
	shader->uniformMatrix4fv("viewMatrix",viewMatrix);
	shader->uniform1f("scalar",3.4f);
	shader->uniform4fv("lightPosition",shiftedLight);
	shader->uniform4fv("lightAmbient",*light_ambient);
	shader->uniform4fv("lightDiffuse",*light_diffuse);
	shader->uniform4fv("translation",trans);
	shader->uniform1i("colorTex",0);
	shader->uniform1i("depthTex",1);
	shader->uniform1i("normTex",2);
	shader->uniform1i("specTex",3);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,textures[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,textures[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,textures[2]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D,textures[3]);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	glBindVertexArray(vertexArrays[0]);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	
	glBindVertexArray(0);
	
	shader->unbind();
	
	glutSwapBuffers();
}


static void key(unsigned char key, int x, int y) {
	switch (key) {
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
double minfps=999.0;
static void idle(void) {
glutPostRedisplay();
return;
	const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	if((t - last) > (1.0/60.0)) {
		last = t;
		glutPostRedisplay();
	}
	if(1.0/(t - last) < minfps)
	{
	        minfps = 1.0/(t - last);
	        cout << "               \r" << int(minfps) << " min FPS\r";
        }
}

int main(int argc, char *argv[]) {
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
	if(err!=GLEW_OK) {
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
	
	struct aiLogStream stream;
	// get a handle to the predefined STDOUT log stream and attach
	// it to the logging system. It will be active for all further
	// calls to aiImportFile(Ex) and aiApplyPostProcessing.
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT,NULL);
	aiAttachLogStream(&stream);
	
	// ... exactly the same, but this stream will now write the
	// log file to assimp_log.txt
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_FILE,"assimp_log.txt");
	aiAttachLogStream(&stream);

	if( 0 != loadasset( argc >= 2 ? argv[1] : "ship.obj")) {
		if( argc != 1 || 0 != loadasset( "../../../test/models-nonbsd/X/ship.obj") && 0 != loadasset( "../../test/models/X/Testwuson.X")) { 
			return -1;
		}
	}
	
	//extract model data from scene
	
	// scale the whole asset to fit into our view frustum 
	float tmp = scene_max.x-scene_min.x;
	tmp = aisgl_max(scene_max.y - scene_min.y,tmp);
	tmp = aisgl_max(scene_max.z - scene_min.z,tmp);
	tmp = 1.f / tmp;
	//can scale by tmp to normalize matrix ie: glScalef(tmp, tmp, tmp);
	
	//can center the model ie: glTranslatef( -scene_center.x, -scene_center.y, -scene_center.z );
	stack<const struct aiNode*> nodeStack;
	nodeStack.push(scene->mRootNode);
	const struct aiNode* node=scene->mRootNode;
	vector<vertex> verts;
	vector<vertex> norms;
	vector<vertex> tans;
	vector<vertex> texvec;
	vertexCount=0;
	while(nodeStack.size() > 0)
	{
		for(unsigned int n=0;n<node->mNumMeshes;++n)
		{
			const struct aiMesh* mesh = scene->mMeshes[node->mMeshes[n]];
			for(unsigned int t=0; t<mesh->mNumFaces;++t)
			{
				const struct aiFace* face = &mesh->mFaces[t];
				for(unsigned int i = 0; i < face->mNumIndices; i++) {
					int index = face->mIndices[i];
					if(mesh->mNormals != NULL)
						norms.push_back(mesh->mNormals[index]);
					if(mesh->HasTextureCoords(0))
						texvec.push_back(mesh->mTextureCoords[0][index]);
					if(mesh->HasTangentsAndBitangents())
						tans.push_back(mesh->mTangents[index]);
					verts.push_back(mesh->mVertices[index]);
					vertexCount++;
				}
			}
		}
		for (unsigned int n = 0; n < node->mNumChildren; ++n) {
			nodeStack.push(node->mChildren[n]);
		}
		node = nodeStack.top();
		nodeStack.pop();
	}
	
	vertices = new vertex[vertexCount];
	normals = new vertex[vertexCount];
	tangents = new vertex[vertexCount];
	texCoords = new vertex[vertexCount];
	for(int i=0;i<vertexCount;i++)
	{
		vertices[i]=verts[i];
		normals[i]=norms[i];
		tangents[i]=tans[i];
		texCoords[i]=texvec[i];
	}
	//calculateModelNormals(normals,4);
	
	//vertex* newNormals = new vertex[vertexCount];
	
	/*for(int i=0;i<vertexCount;i++) {
		float count = 1.0f;
		newNormals[i].x = normals[i].x;
		newNormals[i].y = normals[i].y;
		newNormals[i].z = normals[i].z;
		for(int j=0;j<vertexCount;j++) {
			if(vertices[i].x == vertices[j].x && vertices[i].y == vertices[j].y && vertices[i].z == vertices[j].z) {
				newNormals[i].x += normals[j].x;
				newNormals[i].y += normals[j].y;
				newNormals[i].z += normals[j].z;
				count++;
			}
	}
	newNormals[i].x /= count;
	newNormals[i].y /= count;
	newNormals[i].z /= count;
	}*/
	
	
	/*texCoords = new GLfloat[vertexCount*2];
	for(int i=0;i<vertexCount*2;i+=2) {
		switch(i%8) {
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
	}*/
	checkError();
	glGenVertexArrays(1, vertexArrays);
	checkError();
	glBindVertexArray(vertexArrays[0]);
	checkError();
	glGenBuffers(4, vertexBuffers);
	checkError();
	glBindBuffer(GL_ARRAY_BUFFER,vertexBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount*sizeof(vertex), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER,vertexBuffers[1]);
	checkError();
	glBufferData(GL_ARRAY_BUFFER, vertexCount*sizeof(vertex), texCoords, GL_STATIC_DRAW);
	checkError();
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	checkError();
	glBindBuffer(GL_ARRAY_BUFFER,vertexBuffers[2]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount*sizeof(vertex), normals, GL_STATIC_DRAW);
	checkError();
	glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	checkError();
	glBindBuffer(GL_ARRAY_BUFFER,vertexBuffers[3]);
	glBufferData(GL_ARRAY_BUFFER, vertexCount*sizeof(vertex), tangents, GL_STATIC_DRAW);
	checkError();
	glVertexAttribPointer((GLuint)3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	checkError();
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glBindVertexArray(0);

	checkError();

	shader = new Shader("shader.vert","shader.frag");
	glBindAttribLocation(shader->id(), vertexBuffers[0], "in_Position"); // Bind a constant attribute location for positions of vertices
	glBindAttribLocation(shader->id(), vertexBuffers[1], "in_TexCoords"); // Bind another constant attribute location, this time for color
	glBindAttribLocation(shader->id(), vertexBuffers[2], "in_Normal");
	glBindAttribLocation(shader->id(), vertexBuffers[3], "in_Tangent");

	checkError();
	cout << "allocating texture" <<endl;
	glGenTextures(4,textures);
	checkError();
	cout << "loading image from file" << endl;
	BitMapFile* image = getBMPData("rock01.bmp");
	BitMapFile* depthmap = getBMPData("rock01_height.bmp");
	BitMapFile* normalmap = getBMPData("rock01_norm.bmp");
	BitMapFile* specmap = getBMPData("rock01_spec.bmp");
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
	
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	checkError();
	cout << "setting texture parameters" << endl;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	checkError();
	cout << "loading texture data" << endl;
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB, depthmap->sizeX, depthmap->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, depthmap->data);
	checkError();
	cout << "generating mipmaps" << endl;
	glGenerateMipmap(GL_TEXTURE_2D); //Generate mipmaps now!!!
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	checkError();
	cout << "setting texture parameters" << endl;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	checkError();
	cout << "loading texture data" << endl;
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB, depthmap->sizeX, depthmap->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, normalmap->data);
	checkError();
	cout << "generating mipmaps" << endl;
	glGenerateMipmap(GL_TEXTURE_2D); //Generate mipmaps now!!!
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	checkError();
	cout << "setting texture parameters" << endl;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	checkError();
	cout << "loading texture data" << endl;
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB, depthmap->sizeX, depthmap->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, specmap->data);
	checkError();
	cout << "generating mipmaps" << endl;
	glGenerateMipmap(GL_TEXTURE_2D); //Generate mipmaps now!!!
	
	
	checkError();

	glutMainLoop();
	
	// cleanup - calling 'aiReleaseImport' is important, as the library 
	// keeps internal resources until the scene is freed again. Not 
	// doing so can cause severe resource leaking.
	aiReleaseImport(scene);
	
	// We added a log stream to the library, it's our job to disable it
	// again. This will definitely release the last resources allocated
	// by Assimp.
	aiDetachAllLogStreams();

	return EXIT_SUCCESS;
}
