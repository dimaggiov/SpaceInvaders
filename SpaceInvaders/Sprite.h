#pragma once
#include <cstdint>
class Sprite
{
private:
	size_t width;
	size_t height;
	uint8_t * data;

public: 
	Sprite();
	Sprite(size_t, size_t, uint8_t *);
	~Sprite();

	uint8_t* getData() const;
	void setData(uint8_t*);
	size_t getWidth() const;
	void setWidth(size_t);
	size_t getHeight() const;
	void setHeight(size_t);
};

