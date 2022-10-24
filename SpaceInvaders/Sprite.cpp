#include "Sprite.h"

Sprite::Sprite(size_t width, size_t height)
{
	this->width = width;
	this->height = height;
	this->data = nullptr;
}

Sprite::~Sprite()
{
	delete[] data;
}

uint8_t* Sprite::getData() const
{
	return data;
}

void Sprite::setData(uint8_t* data)
{
	this->data = data;
}

size_t Sprite::getWidth() const
{
	return width;
}

void Sprite::setWidth(size_t width)
{
	this->width = width;
}

size_t Sprite::getHeight() const
{
	return height;
}

void Sprite::setHeight(size_t height)
{
	this->height = height;
}
