#pragma once
#include "Alien.h"
#include "Player.h"
#include "Missile.h"
class Game
{
public:
	static const int MAX_MISSILES = 128;
private:
	size_t width;
	size_t height;
	size_t numMissiles;
	size_t numAliens;
	Alien* aliens;
	Player* player;
	Missile missiles[MAX_MISSILES];

	void initAliens();

public:
	
	Game(size_t, size_t, size_t, Player*);
	Game(size_t, size_t, size_t);
	~Game();

	size_t getWidth();
	void setWidth(size_t);
	size_t getHeight();
	void setHeight(size_t);
	size_t getNumMissiles();
	void fireMissile(size_t, size_t, int);
	void moveMissile(size_t loc);
	Missile* getMissiles();
	void calculateNewMissileLocations();
	size_t getNumAliens();
	Alien* getAlien(size_t);
	size_t getPlayerX();
	void setPlayerX(size_t);
	size_t getPlayerY();
	void setPlayerY(size_t);
	
	
};

