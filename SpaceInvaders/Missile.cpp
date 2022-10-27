#include "Missile.h"

Missile::Missile()
{
	x = 0;
	y = 0;
	dir = 0;
}

Missile::Missile(size_t x, size_t y, int dir)
{
	this->x = x;
	this->y = y;
	this->dir = dir;
}

size_t Missile::getX()
{
	return x;
}

size_t Missile::getY()
{
	return y;
}

int Missile::getDir()
{
	return dir;
}

void Missile::moveWithSpeed(size_t speed)
{
	y += speed * dir;
}
