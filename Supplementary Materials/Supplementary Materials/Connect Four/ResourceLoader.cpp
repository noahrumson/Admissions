// Noah Rubin

#include "ResourceLoader.h"

#include <fstream>
#include <sstream>
#include <unordered_map>

#ifdef _WIN32
#	include <Windows.h>
#	include "resource.h"
#endif

template<>
struct std::hash<VertexData>
{
	std::size_t operator()(const VertexData& arg) const
	{
		const int* verts = reinterpret_cast<const int*>(arg.vertices);
		const int* uvs = reinterpret_cast<const int*>(arg.uvs);
		//return (verts[0] - uvs[0]) * (verts[1] - uvs[1] - verts[2]);
		std::size_t a = std::hash<float>()(arg.vertices[0] + arg.vertices[1] + arg.vertices[2]);
		return a;
	}
};


bool operator==(const VertexData& a, const VertexData& b)
{
	return (a.vertices[0] == b.vertices[0]) && (a.vertices[1] == b.vertices[1]) && (a.vertices[2] == b.vertices[2]);
}

namespace
{
	void IndexBufferObject(
		std::vector<AttributeData>& verts,
		std::vector<AttributeData>& uvs,
		std::vector<GLushort>& indices
		)
	{
		std::vector<AttributeData> outVerts;
		std::vector<AttributeData> outUvs;
		std::unordered_map<VertexData, GLushort> indexMap;
		VertexData data;
		int k = 0;
		for (int i = 0, j = 0; i < verts.size(); i += 3, j += 2) {
			data.vertices = (float*) &verts[i];
			//data.uvs = (float*) &uvs[j];
			data.uvs = 0;
			auto it = indexMap.find(data);
			if (it != indexMap.end()) {
				indices.push_back(it->second); ++k;
			}
			else {
				outVerts.insert(outVerts.end(), data.vertices, data.vertices + 3);
				//outUvs.insert(outUvs.end(), data.uvs, data.uvs + 2);
				GLushort newIndex = outVerts.size() / 3 - 1;
				indices.push_back(newIndex);
				indexMap[data] = newIndex;
			}
		}
		verts.swap(outVerts);
		uvs.swap(outUvs);
	}


	void ReadObjVertex(std::istream& stream, std::vector<GLfloat>& verts)
	{
		GLfloat vertex;
		stream.seekg(2);

		stream >> vertex;
		verts.push_back(vertex);

		stream >> vertex;
		verts.push_back(vertex);

		stream >> vertex;
		verts.push_back(vertex);
	}


	void ReadObjTextureCoord(std::istream& stream, std::vector<GLfloat>& uvs)
	{
		GLfloat vert;
		stream.seekg(2);

		stream >> vert;
		uvs.push_back(vert);

		stream >> vert;
		uvs.push_back(1.0f - vert);
	}


	void ReadObjFace(
		std::istream& stream,
		std::vector<GLushort>& vertIndices,
		std::vector<GLushort>& uvIndices
		)
	{
		GLushort index;
		stream.seekg(2);

		stream >> index;
		vertIndices.push_back(index - 1);
		stream.ignore();
		//stream >> index;
		//uvIndices.push_back(index - 1);



		stream >> index;
		vertIndices.push_back(index - 1);
		stream.ignore();
		//stream >> index;
		//uvIndices.push_back(index - 1);



		stream >> index;
		vertIndices.push_back(index - 1);
		//stream.ignore();
		//stream >> index;
		//uvIndices.push_back(index - 1);
	}


	std::string ReadFile(const std::string& path)
	{
		std::ifstream stream(path);
		stream.seekg(0, std::ios::end);
		std::streamsize size = stream.tellg();
		std::string data(size, 0);
		stream.seekg(0);
		stream.read(&data[0], size);
		return data;
	}


	std::string ReadResource(int id)
	{
		HMODULE instance = GetModuleHandle(NULL);
		HRSRC resource = FindResource(instance, MAKEINTRESOURCE(id), "Shader");
		HGLOBAL loadedResource;
		LPVOID lockedResource;
		if (resource && (loadedResource = LoadResource(instance, resource)) && (lockedResource = LockResource(loadedResource))) {
			DWORD size = SizeofResource(instance, resource);
			const char* str = (const char*) lockedResource;
			return std::string(str, size);
		}
		throw std::runtime_error("Error loading resource");
	}


	GLuint CompileShader(const std::string& data, GLenum type)
	{
		if (data.empty()) {
			return 0;
		}
		const char* src = data.c_str();
		GLuint shaderId = glCreateShader(type);
		glShaderSource(shaderId, 1, &src, 0);
		glCompileShader(shaderId);
		GLint result;
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
		if (result == GL_FALSE) {
			GLint logLength;
			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
			std::string error(logLength, 0);
			glGetShaderInfoLog(shaderId, logLength, 0, &error[0]);
			throw std::runtime_error("Problem loading shader: " + error);
		}
		return shaderId;
	}
}


IndexedMesh MeshFromObjFile(const std::string& file)
{
	VertexAttribute verts(0, 3, GL_FLOAT);
	VertexAttribute uvs(1, 2, GL_FLOAT);
	std::ifstream dataStream(file);
	std::vector<GLushort> indices;
	std::vector<GLfloat> tempVerts;
	std::vector<GLfloat> tempUvs;
	std::vector<GLushort> vertIndices;
	std::vector<GLushort> uvIndices;
	std::string line;
	std::string header;
	std::istringstream lineStream;
	while (std::getline(dataStream, line)) {
		header = line.substr(0, 2);
		lineStream.str(line);
		if (header == "v ") {
			ReadObjVertex(lineStream, tempVerts);
		}
		else if (header == "vt") {
			ReadObjTextureCoord(lineStream, tempUvs);
		}
		else if (header == "f ") {
			ReadObjFace(lineStream, vertIndices, uvIndices);
		}
	}
	for (int i = 0; i < vertIndices.size(); ++i) {
		verts.data.insert(verts.data.end(), &tempVerts[vertIndices[i] * 3], (&tempVerts[vertIndices[i] * 3]) + 3);
	}
	for (int i = 0; i < uvIndices.size(); ++i) {
		uvs.data.insert(uvs.data.end(), &tempUvs[uvIndices[i] * 2], (&tempUvs[uvIndices[i] * 2]) + 2);
	}
	IndexBufferObject(verts.data, uvs.data, indices);
	return IndexedMesh(std::vector<VertexAttribute> { verts }, indices, GL_TRIANGLES);
}


GLuint LoadShaderFile(const std::string& path, GLenum type)
{
	std::string data = ReadFile(path);
	return CompileShader(data, type);
}


GLuint LoadShaderFile(int id, GLenum type)
{
	std::string data = ReadResource(id);
	return CompileShader(data, type);
}


GLuint LinkShaders(GLuint vert, GLuint frag)
{
	GLuint progId = glCreateProgram();
	glAttachShader(progId, vert);
	glAttachShader(progId, frag);
	glLinkProgram(progId);
	GLint result;
	glGetProgramiv(progId, GL_LINK_STATUS, &result);
	if (result == GL_FALSE) {
		GLint logLength;
		glGetProgramiv(progId, GL_INFO_LOG_LENGTH, &logLength);
		std::string error(logLength, 0);
		glGetProgramInfoLog(progId, logLength, 0, &error[0]);
		throw std::runtime_error("Problem linking shader: " + error);
	}
	glDeleteShader(vert);
	glDeleteShader(frag);
	return progId;
}