#pragma once
#include <cstdint>

class Alien
{

private:
	size_t x;
	size_t y;
	uint8_t type;

public:
	Alien();
	Alien(size_t, size_t, uint8_t);

	size_t getX();
	void setX(size_t);
	size_t getY();
	void setY(size_t);
	uint8_t getType();
	
	
};

