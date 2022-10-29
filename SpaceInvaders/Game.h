#pragma once
#include "Alien.h"
#include "Player.h"
#include "Missile.h"
#include "Sprite.h"
#include "Bunker.h"
#include "Buffer.h"
#include "Sprites.h"


class Game
{
public:
	static const int MAX_MISSILES = 128;
	static const int PLAYER_MAX_MISSILES = 2;
private:

	bool gameOver;
	bool continueRound;
	size_t width;
	size_t height;
	size_t score;

	size_t numMissiles;
	size_t numAliens;
	size_t numDeadAliens;
	uint8_t* deathCounters;
	size_t numBunkers;
	

	size_t chanceToShoot;
	size_t alienMoveSpeed;
	int alienMoveDirection;
	int playerMoveDirection;

	Alien* aliens;
	Player* player;
	Bunker* bunkers;
	Missile missiles[MAX_MISSILES];

	void initBunkers();
	void initAliens();
	void resetAlienPositions();

	void playerFireMissile(size_t, size_t, int);
	void alienFireMissile(Alien*, size_t, size_t, int);
	void moveMissile(size_t loc);
	void removeMissile(size_t loc);
	size_t getPlayerMissileCount();

	bool checkAlienHit(Sprite*, size_t, Sprite*, size_t);
	bool checkPlayerHit(Sprite*, size_t, Sprite*);
	bool checkBunkerHit(Sprite*, size_t, Sprite*, size_t);
	bool checkAlienHitBunker(Sprite*, size_t, Sprite*, size_t);
	bool checkAlienHitPlayer(Sprite*, size_t, Sprite*);

public:
	
	Game(size_t, size_t, size_t, Player*);
	Game(size_t, size_t, size_t);
	~Game();

	size_t getWidth();
	size_t getHeight();
	



	void shootForPlayer();
	void shootAlienMissiles();
	void checkCollisions();

	void movePlayer(int);
	void moveAliens(unsigned int);

	void drawGame(Buffer*);
	void startNewRound();
	bool isGameOver();
	bool shouldContinueRound();
	void setContinueRound(bool);
	
	
};

