// Noah Rubin

#include "Mesh.h"


IndexedMesh::IndexedMesh(
	const std::vector<VertexAttribute>& attribs,
	const std::vector<GLushort>& indices,
	GLenum primitive
	) :
	IndexedMesh(attribs, indices, primitive, true)
{
}

IndexedMesh::IndexedMesh(IndexedMesh&& move) :
	VertexMesh(std::move(move)),
	m_indexBuffer(move.m_indexBuffer)
{
	move.m_indexBuffer = 0;
}

IndexedMesh& IndexedMesh::operator=(IndexedMesh&& move)
{
	VertexMesh::operator=(std::move(move));
	m_indexBuffer = move.m_indexBuffer;
	move.m_indexBuffer = 0;
	return *this;
}


IndexedMesh::IndexedMesh(
	const std::vector<VertexAttribute>& attribs,
	const std::vector<GLushort>& indices,
	GLenum primitive,
	bool unbind
	) :
	VertexMesh(attribs, primitive, indices.size(), false),
	m_indexBuffer(0)
{
	glGenBuffers(1, &m_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);
	if (unbind) {
		glBindVertexArray(0);
	}
}


IndexedMesh::IndexedMesh(
	const std::vector<VertexAttribute>& attribs,
	const std::vector<GLushort>& indices,
	GLenum primitive,
	bool unbind,
	GLsizei numVerts
	) :
	VertexMesh(attribs, primitive, numVerts, false),
	m_indexBuffer(0)
{
	glGenBuffers(1, &m_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);
	if (unbind) {
		glBindVertexArray(0);
	}
}


IndexedMesh::~IndexedMesh()
{
	glDeleteBuffers(1, &m_indexBuffer);
}


void IndexedMesh::Render() const
{
	glBindVertexArray(GetVao());
	glDrawElements(GetPrimitive(), GetNumIndices(), GL_UNSIGNED_SHORT, nullptr);
	glBindVertexArray(0);
}