// Noah Rubin

#include "Mesh.h"

InstancedVertexMesh::InstancedVertexMesh(
	const std::vector<VertexAttribute>& attribs,
	GLenum primitive,
	GLsizei numInstances
	) :
	VertexMesh(attribs, primitive),
	m_numInstances(numInstances)
{
}


void InstancedVertexMesh::Render() const
{
	glBindVertexArray(GetVao());
	glDrawArraysInstanced(GetPrimitive(), 0, GetNumVerts(), m_numInstances);
	glBindVertexArray(0);
}