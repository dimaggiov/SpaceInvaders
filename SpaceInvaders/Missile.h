#pragma once
class Missile
{
private:
	size_t x;
	size_t y;
	int dir;

public:
	Missile();
	Missile(size_t, size_t, int);

	size_t getX();
	size_t getY();
	int getDir();
	void moveWithSpeed(size_t);



};

