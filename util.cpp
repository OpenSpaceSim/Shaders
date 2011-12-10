#include "util.h"
#include <inttypes.h>
using namespace std;

void get_bounding_box_for_node (const struct aiNode* nd, 
	struct aiVector3D* min, 
	struct aiVector3D* max, 
	struct aiMatrix4x4* trafo
){
	struct aiMatrix4x4 prev;
	unsigned int n = 0, t;

	prev = *trafo;
	aiMultiplyMatrix4(trafo,&nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {

			struct aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp,trafo);

			min->x = aisgl_min(min->x,tmp.x);
			min->y = aisgl_min(min->y,tmp.y);
			min->z = aisgl_min(min->z,tmp.z);

			max->x = aisgl_max(max->x,tmp.x);
			max->y = aisgl_max(max->y,tmp.y);
			max->z = aisgl_max(max->z,tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n],min,max,trafo);
	}
	*trafo = prev;
}

void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max)
{
	struct aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene->mRootNode,min,max,&trafo);
}

int loadasset (const char* path)
{
	// we are taking one of the postprocessing presets to avoid
	// writing 20 single postprocessing flags here.
	scene = aiImportFile(path,aiProcessPreset_TargetRealtime_Quality);

	if (scene) {
		get_bounding_box(&scene_min,&scene_max);
		scene_center.x = (scene_min.x + scene_max.x) / 2.0f;
		scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
		scene_center.z = (scene_min.z + scene_max.z) / 2.0f;
		return 0;
	}
	return 1;
}

// Routine to read a bitmap file.
// Works only for uncompressed bmp files of 24-bit color.
BitMapFile *getBMPData(string filename) {
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
	infile.seekg(28);
	infile.read( (char *) &bmp->bitDepth, 2);
	if (bmp->bitDepth !=24 && bmp->bitDepth!=32) {
		cout << "ERROR - Cannot load bitmap. Unknown bitdepth: " << bmp->bitDepth << endl;
		delete bmp;
		return 0;
	}

	// Allocate buffer for the image.
	size = bmp->sizeX * bmp->sizeY * bmp->bitDepth;
	bmp->data = new unsigned char[size];

	// Read bitmap data.
	infile.seekg(offset);
	infile.read((char *) bmp->data , size);

	// Reverse color from bgr to rgb.
	if (bmp->bitDepth!=32) {
		uint32_t temp;
		for (int i = 0; i < size; i += 3) { 
			temp = bmp->data[i];
			bmp->data[i] = bmp->data[i+2];
			bmp->data[i+2] = temp;
		}
	} else {
		uint32_t R, G, B, A;
		for (int i = 0; i < size; i += 4) {
			A = bmp->data[i];
			R = bmp->data[i+1];
			G = bmp->data[i+2];
			B = bmp->data[i+3];
			bmp->data[i+2] = R;
			bmp->data[i+1] = G;
			bmp->data[i+0] = B;
			bmp->data[i+3] = A;
		}
	}
	return bmp;
}

void checkError() {
	GLenum err = glGetError();
	if(err == GL_NO_ERROR) return;
	cout << gluErrorString(err);
	if(err == GL_INVALID_ENUM) { 
		cout <<" (GL_INVALID_ENUM)" << endl;
		return;
	}
	if(err == GL_INVALID_VALUE) {
		cout << " (GL_INVALID_VALUE)"<<endl;
		return;
	}
	if(err == GL_INVALID_OPERATION) {
		cout << " (GL_INVALID_OPERATION)"<<endl;
		return;
	}
	cout << " (" << err << ")" << endl;
}

//calculates average surface normal for arbitrary polygon
void calculateSurfaceNormal(const GLfloat* polygon, GLint count, GLfloat* normal) {
	normal[0] = 0.0f;
	normal[1] = 0.0f;
	normal[2] = 0.0f;
	for(int i=0;i<(count*3);i+=3) {
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
void calculateModelNormals(vertex* normals, GLint polygonSize) {
	GLfloat normal[3];
	GLfloat* polygon = (GLfloat*)malloc(3 * sizeof(GLfloat*) * polygonSize);
	for(int i=0; (i+polygonSize)<=vertexCount; i+= polygonSize) {
		//cout << "i=" << i << endl;
		int k=0;
		for(int j=0;j<3*polygonSize;j+=3) {
			if(!((i+j+2)<(vertexCount*3))) {
				cout << "ERROR:vertices not even multiple of polygon size\n";
				return;
			}
			polygon[j] = vertices[i+k].x;
			polygon[j+1] = vertices[i+k].y;
			polygon[j+2] = vertices[i+k].z;
			k++;
		}
		calculateSurfaceNormal(polygon,polygonSize,normal);
		for(int j=0;j<polygonSize;j++) {
			normals[i+j].x = normal[0];
			normals[i+j].y = normal[1];
			normals[i+j].z = normal[2];
		}
	}
}

void loadModel() {
	ifstream file("ncc1701.data");
	if(!file.is_open()) {
		cout << "unable to open file \n";
		return;
	}
	vector<vertex> vertvec;
	while(!file.eof()) {
		vertex point;
		file >> point.x;
		file >> point.y;
		file >> point.z;
		vertvec.push_back(point);
	}
	vertices = new vertex[vertvec.size()];
	for(int i=0;i<vertvec.size();i++) {
		vertices[i].x = vertvec[i].x;
		vertices[i].y = vertvec[i].y;
		vertices[i].z = vertvec[i].z;
	}
	vertexCount = vertvec.size();
}

void BuildPerspProjMat(float *m, float fov, float aspect, float znear, float zfar) {
	float xymax = znear * tan(fov * (3.1415926535897932384626/360.0));
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
bool LoadTextureFromBitmap(const char *file, GLuint texture) {
	cout << "loading image from file: " << file << endl;
	BitMapFile* image = getBMPData(file);
	if (image==0) {
		return false;
	}
	glBindTexture(GL_TEXTURE_2D, texture);
	checkError();
	cout << "setting texture parameters" << endl;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	checkError();
	cout << "loading texture data" << endl;
	if (image->bitDepth==24) {
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB, image->sizeX, image->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
	} else {
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB, image->sizeX, image->sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
	}
	checkError();
	cout << "generating mipmaps" << endl;
	glGenerateMipmap(GL_TEXTURE_2D); //Generate mipmaps now!!!
	delete image;
	return true;
}
