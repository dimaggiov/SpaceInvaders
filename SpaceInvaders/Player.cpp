#include "Player.h"

Player::Player()
{
	x = 107;
	y = 20;
	life = 3;
}

size_t Player::getX()
{
	return x;
}

void Player::setX(size_t x)
{
	this->x = x;
}

size_t Player::getY()
{
	return y;
}

void Player::setY(size_t y)
{
	this->y = y;
}

void Player::died()
{
	life--;
}
