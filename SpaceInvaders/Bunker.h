#pragma once
class Bunker
{
private:
	size_t x;
	size_t y;
	size_t life;

public:
	Bunker();
	Bunker(size_t, size_t);

	size_t getX();
	void setX(size_t);
	size_t getY();
	void setY(size_t);
	size_t getlife();
	void takeDamage();
	
};

