// Noah Rubin

#include "Mesh.h"
#include "VertexAttributeHelpers.h"

VertexMesh::VertexMesh(const std::vector<VertexAttribute>& attribs, GLenum primitive) :
VertexMesh(attribs, primitive, attribs[0].data.size() / attribs[0].attribSize, true)
{
}


VertexMesh::VertexMesh(
	const std::vector<VertexAttribute>& attribs,
	GLenum primitive,
	GLsizei numVerts,
	bool unbind
	) :
	m_attribBuffers(attribs.size()),
	m_vao(0),
	m_primitive(primitive),
	m_numVerts(numVerts)
{
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	glGenBuffers(attribs.size(), m_attribBuffers.data());
	for (int i = 0; i < attribs.size(); ++i) {
		const VertexAttribute& attrib = attribs[i];
		glBindBuffer(GL_ARRAY_BUFFER, m_attribBuffers[i]);
		WriteBuffer(attrib, attrib.type);
		glEnableVertexAttribArray(attrib.index);
		if (IsIntegral(attrib)) {
			glVertexAttribIPointer(attrib.index, attrib.attribSize, attrib.type, 0, 0);
		}
		else {
			glVertexAttribPointer(attrib.index, attrib.attribSize, attrib.type, attrib.normalized, 0, 0);
		}
	}
	if (unbind) {
		glBindVertexArray(0);
	}
}


VertexMesh::VertexMesh(VertexMesh&& move) :
	m_attribBuffers(std::move(move.m_attribBuffers)),
	m_vao(move.m_vao),
	m_primitive(move.m_primitive),
	m_numVerts(move.m_numVerts)
{
	move.m_vao = 0;
}


VertexMesh& VertexMesh::operator=(VertexMesh&& move)
{
	m_attribBuffers = std::move(move.m_attribBuffers);
	m_vao = move.m_vao;
	m_primitive = move.m_primitive;
	m_numVerts = move.m_numVerts;
	move.m_vao = 0;
	return *this;
}


VertexMesh::~VertexMesh()
{
	if (m_attribBuffers.size()) {
		glDeleteBuffers(m_attribBuffers.size(), m_attribBuffers.data());
		glDeleteVertexArrays(1, &m_vao);
	}
}


void VertexMesh::Render() const
{
	glBindVertexArray(m_vao);
	glDrawArrays(m_primitive, 0, m_numVerts);
	glBindVertexArray(0);
}