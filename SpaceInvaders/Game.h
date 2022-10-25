#pragma once
#include "Alien.h"
#include "Player.h"
class Game
{
private:
	size_t width;
	size_t height;
	size_t numAliens;
	Alien* aliens;
	Player* player;

	void initAliens();

public:
	Game(size_t, size_t, size_t, Player*);
	Game(size_t, size_t, size_t);
	~Game();

	size_t getWidth();
	void setWidth(size_t);
	size_t getHeight();
	void setHeight(size_t);
	size_t getNumAliens();
	Alien* getAlien(size_t);
	size_t getPlayerX();
	void setPlayerX(size_t);
	size_t getPlayerY();
	void setPlayerY(size_t);
	
	
};

