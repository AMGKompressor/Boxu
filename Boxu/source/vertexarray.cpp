// COMP710 GP Framework 2025

#include "vertexarray.h"

#include <GL/glew.h>
#include <cassert>

VertexArray::VertexArray(const float* vertexData, unsigned int numVertices, const unsigned int* indexData, unsigned int numIndices)
	: mNumVertices(numVertices)
	, mNumIndices(numIndices)
	, mGlVertexBuffer(0)
	, mGlIndexBuffer(0)
	, mGlVertexArray(0)
{
	const int stride = 5 * sizeof(float);

	assert(vertexData);

	glGenVertexArrays(1, &mGlVertexArray);
	glBindVertexArray(mGlVertexArray);

	glGenBuffers(1, &mGlVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mGlVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, mNumVertices * stride, vertexData, GL_STATIC_DRAW);

	if (indexData)
	{
		glGenBuffers(1, &mGlIndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGlIndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mNumIndices * sizeof(unsigned int), indexData, GL_STATIC_DRAW);
	}

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(sizeof(float) * 3));
}

VertexArray::~VertexArray()
{
	glDeleteBuffers(1, &mGlVertexBuffer);
	glDeleteBuffers(1, &mGlIndexBuffer);
	glDeleteVertexArrays(1, &mGlVertexArray);
}

void VertexArray::setActive()
{
	glBindVertexArray(mGlVertexArray);
}

unsigned int VertexArray::getNumVertices() const
{
	return mNumVertices;
}

unsigned int VertexArray::getNumIndices() const
{
	return mNumIndices;
}
