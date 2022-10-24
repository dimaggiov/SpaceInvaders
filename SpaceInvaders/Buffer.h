#pragma once
#include<cstdio>
#include<cstdint>
#include "Sprite.h"


class Buffer
{
private: 
	size_t width;
	size_t height;
	uint32_t* data;

public:
	Buffer(size_t, size_t);
	~Buffer();
	uint32_t * getData() const;
	void setData(uint32_t *);

	const static uint32_t rgb_to_uint(uint8_t r, uint8_t g, uint8_t b);
	void clearBuffer(uint32_t);
	void drawSprite(const Sprite&, size_t, size_t, uint32_t);


	size_t getWidth() const;
	void setWidth(size_t);
	size_t getHeight() const;
	void setHeight(size_t);
	
};


