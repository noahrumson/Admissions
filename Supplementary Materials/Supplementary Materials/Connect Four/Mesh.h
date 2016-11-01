// Noah Rubin

#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED

#include "Libraries/gl/glew.h"

#include <vector>
#include <utility>

union AttributeData
{
	AttributeData();
	explicit AttributeData(GLfloat f);
	explicit AttributeData(GLint i);
	explicit AttributeData(GLuint ui);
	explicit AttributeData(GLshort s);
	explicit AttributeData(GLushort us);
	explicit AttributeData(GLbyte b);
	explicit AttributeData(GLubyte ub);

	GLfloat fVal;
	GLint iVal;
	GLuint uiVal;
	GLshort sVal;
	GLushort usVal;
	GLbyte bVal;
	GLubyte ubVal;
};


struct VertexAttribute
{
	VertexAttribute();

	VertexAttribute(
		GLuint index,
		GLint attribSize,
		GLenum type,
		bool normalized = false
		);

	VertexAttribute(
		const std::vector<AttributeData>& data,
		GLuint index,
		GLint attribSize,
		GLenum type,
		bool normalized = false
		);

	std::vector<AttributeData> data;
	GLuint index;
	GLint attribSize;
	GLenum type;
	bool normalized;

	template<class T>
	void AddData(T* data, int size);

	template<class T>
	void AddData(const std::vector<T>& data);
};


class IMesh
{
public:
	virtual ~IMesh() = default;

	virtual void Render() const = 0;
};


class VertexMesh : public IMesh
{
public:
	VertexMesh(const std::vector<VertexAttribute>& attribs, GLenum primitive);

	VertexMesh(const VertexMesh& copy) = delete;

	VertexMesh(VertexMesh&& move);

	VertexMesh& operator=(const VertexMesh& copy) = delete;

	VertexMesh& operator=(VertexMesh&& move);

	virtual ~VertexMesh();

	virtual void Render() const override;

protected:
	VertexMesh(
		const std::vector<VertexAttribute>& attribs,
		GLenum primitive,
		GLsizei numVerts,
		bool unbind
		);

	const std::vector<GLuint>& GetAttribBuffers() const;

	GLuint GetVao() const;

	GLenum GetPrimitive() const;

	GLuint GetNumVerts() const;

private:
	std::vector<GLuint> m_attribBuffers;
	GLuint m_vao;
	GLenum m_primitive;
	GLsizei m_numVerts;
};


class IndexedMesh : public VertexMesh
{
public:
	IndexedMesh(
		const std::vector<VertexAttribute>& attribs,
		const std::vector<GLushort>& indices,
		GLenum primitive
	);

	IndexedMesh(IndexedMesh&& move);

	IndexedMesh& operator=(IndexedMesh&& move);

	virtual ~IndexedMesh();

	virtual void Render() const override;

protected:
	GLuint GetNumIndices() const;

	IndexedMesh(
		const std::vector<VertexAttribute>& attribs,
		const std::vector<GLushort>& indices,
		GLenum primitive,
		bool unbind,
		GLsizei numVerts
		);

	IndexedMesh(
		const std::vector<VertexAttribute>& attribs,
		const std::vector<GLushort>& indices,
		GLenum primitive,
		bool unbind
		);

private:
	GLuint m_indexBuffer;
};


class InstancedVertexMesh : public VertexMesh
{
public:
	InstancedVertexMesh(
		const std::vector<VertexAttribute>& attribs,
		GLenum primitive,
		GLsizei numInstances
	);

	virtual void Render() const override;

	void SetInstances(GLsizei num);

private:
	GLsizei m_numInstances;
};


inline const std::vector<GLuint>& VertexMesh::GetAttribBuffers() const
{
	return m_attribBuffers;
}


inline GLuint VertexMesh::GetVao() const
{
	return m_vao;
}


inline GLenum VertexMesh::GetPrimitive() const
{
	return m_primitive;
}


inline GLenum VertexMesh::GetNumVerts() const
{
	return m_numVerts;
}


inline GLuint IndexedMesh::GetNumIndices() const
{
	return GetNumVerts();
}


inline void InstancedVertexMesh::SetInstances(GLsizei num)
{
	m_numInstances = num;
}


template<class T>
inline void VertexAttribute::AddData(T* dat, int size)
{
	for (T* it = dat; it != dat + size; ++it) {
		data.push_back(AttributeData(*it));
	}
}

template<class T>
inline void VertexAttribute::AddData(const std::vector<T>& dat)
{
	data.insert(data.end(), dat.begin(), dat.end());
}

#endif // !MESH_H_INCLUDED
