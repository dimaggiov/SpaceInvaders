#include "Bunker.h"

Bunker::Bunker()
{
	this->x = 0;
	this->y = 0;
	this->life = 0;
}

Bunker::Bunker(size_t x, size_t y)
{
	this->x = x;
	this->y = y;
	this->life = 3;
}

size_t Bunker::getX()
{
	return x;
}

void Bunker::setX(size_t x)
{
	this->x = x;
}

size_t Bunker::getY()
{
	return y;
}

void Bunker::setY(size_t y)
{
	this->y = y;
}

size_t Bunker::getlife()
{
	return life;
}

void Bunker::takeDamage()
{
	life--;
}
