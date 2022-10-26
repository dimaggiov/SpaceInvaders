#pragma once
#include <cstdint>
#include "AlienType.h"

class Alien
{

private:
	size_t x;
	size_t y;
	AlienType type;

public:
	Alien();
	Alien(size_t, size_t, AlienType);

	size_t getX();
	void setX(size_t);
	size_t getY();
	void setY(size_t);
	AlienType getType();
	
	
};

