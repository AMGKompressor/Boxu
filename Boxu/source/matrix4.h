// COMP710 GP Framework 2024
#ifndef __MATRIX4_H_
#define __MATRIX4_H_

struct Matrix4
{
	float m[4][4];
};

void setZero(Matrix4& mat);
void setIdentity(Matrix4& mat);
void createOrthoProjection(Matrix4& mat, float width, float height);
void createTranslation(Matrix4& mat, float tx, float ty);
void matrixMultiply(Matrix4& out, const Matrix4& a, const Matrix4& b);

#endif // __MATRIX4_H_
