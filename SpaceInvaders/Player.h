#pragma once
class Player
{
private: 
	size_t x;
	size_t y;
	size_t life;

public:
	Player();

	size_t getX();
	void setX(size_t);
	size_t getY();
	void setY(size_t);
	size_t getLives();
	void died();
};

