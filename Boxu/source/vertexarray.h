// COMP710 GP Framework 2024
#ifndef __VERTEXTARRAY_H_
#define __VERTEXTARRAY_H_

class VertexArray
{
public:
	VertexArray(const float* vertices, unsigned int numVertices, const unsigned int* indices, unsigned int numIndices);
	~VertexArray();

	void setActive();

	unsigned int getNumVertices() const;
	unsigned int getNumIndices() const;

private:
	VertexArray(const VertexArray& vertexArray);
	VertexArray& operator=(const VertexArray& vertexArray);

	unsigned int mNumVertices;
	unsigned int mNumIndices;

	unsigned int mGlVertexBuffer;
	unsigned int mGlIndexBuffer;
	unsigned int mGlVertexArray;
};

#endif // __VERTEXTARRAY_H_
