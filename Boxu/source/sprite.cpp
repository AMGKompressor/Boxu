// COMP710 GP Framework 2025

#include "sprite.h"

#include <cmath>

#include "renderer.h"
#include "texture.h"

Sprite::Sprite()
	: mTexture(0)
	, mX(0)
	, mY(0)
	, mBaseWidth(0)
	, mBaseHeight(0)
	, mAngle(0.0f)
	, mCenterX(0)
	, mCenterY(0)
	, mScale(1.0f)
	, mAlpha(1.0f)
	, mTintRed(1.0f)
	, mTintGreen(1.0f)
	, mTintBlue(1.0f)
{

}

Sprite::~Sprite()
{

}

bool Sprite::initialize(Texture& texture)
{
	mTexture = &texture;

	mBaseWidth = mTexture->getWidth();
	mBaseHeight = mTexture->getHeight();

	return true;
}

void Sprite::process(float deltaTime)
{
	(void)deltaTime;
}

void Sprite::draw(Renderer& renderer)
{
	mTexture->setActive();
	renderer.drawSprite(*this);
}

void Sprite::setX(int x)
{
	mX = x;
}

void Sprite::setY(int y)
{
	mY = y;
}

int Sprite::getX() const
{
	return mX;
}

int Sprite::getY() const
{
	return mY;
}

void Sprite::setAngle(float angle)
{
	while (angle >= 360.0f)
	{
		angle -= 360.0f;
	}
	while (angle < 0.0f)
	{
		angle += 360.0f;
	}

	mAngle = angle;
}

float Sprite::getAngle() const
{
	return mAngle;
}

void Sprite::setScale(float scale)
{
	mScale = scale;
}

float Sprite::getScale() const
{
	return mScale;
}

void Sprite::setAlpha(float alpha)
{
	mAlpha = clamp(0.0f, alpha, 1.0f);
}

float Sprite::getAlpha() const
{
	return mAlpha;
}

int Sprite::getWidth() const
{
	return static_cast<int>(ceilf(static_cast<float>(mBaseWidth) * mScale));
}

int Sprite::getHeight() const
{
	return static_cast<int>(ceilf(static_cast<float>(mBaseHeight) * mScale));
}

float Sprite::clamp(float minimum, float value, float maximum)
{
	if (value > maximum)
	{
		value = maximum;
	}
	else if (value < minimum)
	{
		value = minimum;
	}

	return value;
}

void Sprite::setRedTint(float value)
{
	mTintRed = clamp(0.0f, value, 1.0f);
}

float Sprite::getRedTint() const
{
	return mTintRed;
}

void Sprite::setGreenTint(float value)
{
	mTintGreen = clamp(0.0f, value, 1.0f);
}

float Sprite::getGreenTint() const
{
	return mTintGreen;
}

void Sprite::setBlueTint(float value)
{
	mTintBlue = clamp(0.0f, value, 1.0f);
}

float Sprite::getBlueTint() const
{
	return mTintBlue;
}
