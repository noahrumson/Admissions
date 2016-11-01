// Noah Rubin

#include "VertexAttributeHelpers.h"

#define WRITE_BUFFER_IMPL_HELPER(type, val)	\
	template<> void WriteBufferImpl<type>(const VertexAttribute& attrib)	\
				{	\
		std::vector<type> temp(attrib.data.size());	\
		for (const AttributeData& dat : attrib.data) {	\
			temp.push_back(dat.val);			\
						}	\
		glBufferData(GL_ARRAY_BUFFER, sizeof(type) * temp.size(), temp.data(), GL_STATIC_DRAW);	\
				}

#define ATTRIBUTE_DATA_CONSTRUCTOR_HELPER(type, val)	\
	AttributeData::AttributeData(type v) :	\
		val(v)	\
								{}

namespace
{
	typedef void(*WriteBufferFunc)(const VertexAttribute&);

	template<class T>
	void WriteBufferImpl(const VertexAttribute& attrib);

	WRITE_BUFFER_IMPL_HELPER(GLbyte, bVal)
	WRITE_BUFFER_IMPL_HELPER(GLubyte, ubVal)
	WRITE_BUFFER_IMPL_HELPER(GLshort, sVal)
	WRITE_BUFFER_IMPL_HELPER(GLushort, usVal)

	template<class T>
	void WriteBufferNoCopyImpl(const VertexAttribute& attrib)
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(T) * attrib.data.size(), attrib.data.data(), GL_STATIC_DRAW);
	}
}


void WriteBuffer(const VertexAttribute& attrib, GLenum type)
{
	static WriteBufferFunc funcs[] {
		&WriteBufferImpl<GLbyte>,
		&WriteBufferImpl<GLubyte>,
		&WriteBufferImpl<GLshort>,
		&WriteBufferImpl<GLushort>,
		&WriteBufferNoCopyImpl<GLint>,
		&WriteBufferNoCopyImpl<GLuint>,
		&WriteBufferNoCopyImpl<GLfloat>
	};
	funcs[type - GL_BYTE](attrib);
}


bool IsIntegral(const VertexAttribute& attrib)
{
	return !(attrib.normalized || attrib.type == GL_FLOAT);
}


AttributeData::AttributeData() :
fVal(0)
{
}

ATTRIBUTE_DATA_CONSTRUCTOR_HELPER(GLfloat, fVal)
ATTRIBUTE_DATA_CONSTRUCTOR_HELPER(GLint, iVal)
ATTRIBUTE_DATA_CONSTRUCTOR_HELPER(GLuint, uiVal)
ATTRIBUTE_DATA_CONSTRUCTOR_HELPER(GLshort, sVal)
ATTRIBUTE_DATA_CONSTRUCTOR_HELPER(GLushort, usVal)
ATTRIBUTE_DATA_CONSTRUCTOR_HELPER(GLbyte, bVal)
ATTRIBUTE_DATA_CONSTRUCTOR_HELPER(GLubyte, ubVal)

VertexAttribute::VertexAttribute() :
VertexAttribute(0, 0, 0, 0)
{
}


VertexAttribute::VertexAttribute(
	GLuint index,
	GLint attribSize,
	GLenum type,
	bool normalized
	) :
	data(),
	index(index),
	attribSize(attribSize),
	type(type),
	normalized(normalized)
{
}


VertexAttribute::VertexAttribute(
	const std::vector<AttributeData>& data,
	GLuint index,
	GLint attribSize,
	GLenum type,
	bool normalized
	) :
	data(data),
	index(index),
	attribSize(attribSize),
	type(type),
	normalized(normalized)
{
}


#undef WRITE_BUFFER_IMPL_HELPER
#undef ATTRIBUTE_DATA_CONSTRUCTOR_HELPER
