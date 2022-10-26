#include "Alien.h"

Alien::Alien()
{
	x = 0;
	y = 0;
	type = ALIEN_DEAD;
}

Alien::Alien(size_t x, size_t y, AlienType type)
{
	this->x = x;
	this->y = y;
	this->type = type;
}

size_t Alien::getX()
{
	return x;
}

void Alien::setX(size_t x)
{
	this->x = x;
}

size_t Alien::getY()
{
	return y;
}

void Alien::setY(size_t y)
{
	this->y = y;
}

AlienType Alien::getType()
{
	return type;
}
