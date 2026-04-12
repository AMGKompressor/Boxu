// COMP710 GP Framework 2024
#include "matrix4.h"

void setZero(Matrix4& mat)
{
	mat.m[0][0] = 0.0f;
	mat.m[0][1] = 0.0f;
	mat.m[0][2] = 0.0f;
	mat.m[0][3] = 0.0f;

	mat.m[1][0] = 0.0f;
	mat.m[1][1] = 0.0f;
	mat.m[1][2] = 0.0f;
	mat.m[1][3] = 0.0f;

	mat.m[2][0] = 0.0f;
	mat.m[2][1] = 0.0f;
	mat.m[2][2] = 0.0f;
	mat.m[2][3] = 0.0f;

	mat.m[3][0] = 0.0f;
	mat.m[3][1] = 0.0f;
	mat.m[3][2] = 0.0f;
	mat.m[3][3] = 0.0f;
}

void setIdentity(Matrix4& mat)
{
	setZero(mat);

	mat.m[0][0] = 1.0f;
	mat.m[1][1] = 1.0f;
	mat.m[2][2] = 1.0f;
	mat.m[3][3] = 1.0f;
}

void createOrthoProjection(Matrix4& mat, float width, float height)
{
	setZero(mat);

	mat.m[0][0] = 2.0f / width;
	mat.m[1][1] = 2.0f / -height;
	mat.m[2][2] = -2.0f / (1.0f - -1.0f);

	mat.m[3][0] = -1.0f;
	mat.m[3][1] = 1.0f;
	mat.m[3][2] = 0.0f;
	mat.m[3][3] = 1.0f;
}

void createTranslation(Matrix4& mat, float tx, float ty)
{
	setIdentity(mat);
	mat.m[3][0] = tx;
	mat.m[3][1] = ty;
}

void matrixMultiply(Matrix4& out, const Matrix4& a, const Matrix4& b)
{
	Matrix4 r;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			r.m[i][j] =
				a.m[i][0] * b.m[0][j] +
				a.m[i][1] * b.m[1][j] +
				a.m[i][2] * b.m[2][j] +
				a.m[i][3] * b.m[3][j];
		}
	}
	out = r;
}
