// Noah Rubin

#ifndef VERTEX_ATTRIBUTE_HELPERS_H_INCLUDED
#define VERTEX_ATTRIBUTE_HELPERS_H_INCLUDED

#include "Mesh.h"

void WriteBuffer(const VertexAttribute& attrib, GLenum type);

bool IsIntegral(const VertexAttribute& attrib);

#endif
