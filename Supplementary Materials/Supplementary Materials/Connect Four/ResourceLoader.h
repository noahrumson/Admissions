// Noah Rubin

#ifndef RESOURCE_LOADER_H_INCLUDED
#define RESOURCE_LOADER_H_INCLUDED

#include <string>
#include <cstdint>

#include "Mesh.h"

IndexedMesh MeshFromObjFile(const std::string& file);

GLuint LoadShaderFile(const std::string& path, GLenum type);

GLuint LoadShaderFile(int id, GLenum type);

GLuint LinkShaders(GLuint vert, GLuint frag);

struct VertexData
{
	const float* vertices;
	const float* uvs;
};

#endif
