#include "Game.h"

void Game::initBunkers()
{
	bunkers = new Bunker[4];
	for (int i = 0; i < 4; i++)
	{
		bunkers[i] = *new Bunker(i * 50 + 20, 30);
	}
}

//give create all aliens and give them initial location
void Game::initAliens()
{
	aliens = new Alien[numAliens];
	for (size_t yi = 0; yi < 5; yi++)
	{
		for (size_t xi = 0; xi < 11; xi++)
		{
			size_t x = 16 * xi + 20;
			size_t y = 17 * yi + 128;
			AlienType t = (AlienType)((5 - yi) / 2 + 1);
			
			aliens[yi * 11 + xi] = *new Alien(x, y, t);
		}
	}
}

Game::Game(size_t width, size_t height, size_t numAliens, Player* player)
{
	this->width = width;
	this->height = height;
	this->score = 0;
	this->numAliens = numAliens;
	this->player = player;
	initAliens();
	initBunkers();
}

Game::Game(size_t width, size_t height, size_t numAliens)
{
	this->width = width;
	this->height = height;
	this->numAliens = numAliens;
	this->score = 0;
	this->player = new Player();
	initAliens();
	initBunkers();
}

Game::~Game()
{
	delete[] aliens;
	delete player;
}

size_t Game::getWidth()
{
	return width;
}

void Game::setWidth(size_t width)
{
	this->width = width;
}

size_t Game::getHeight()
{
	return height;
}

void Game::setHeight(size_t height)
{
	this->height = height;
}

size_t Game::getScore()
{
	return score;
}

void Game::addScore(size_t score)
{
	this->score += score;
}

size_t Game::getNumMissiles()
{
	return numMissiles;
}

void Game::fireMissile(size_t width, size_t height, int dir)
{
	missiles[numMissiles] = *new Missile(player->getX() + width / 2, player->getY() + height, dir);
	numMissiles++;
}

void Game::removeMissile(size_t loc)
{
	missiles[loc] = missiles[numMissiles - 1];
	numMissiles--;
}

void Game::moveMissile(size_t loc)
{
	missiles[loc].moveWithSpeed(1);
}

Missile* Game::getMissiles()
{
	return missiles;
}

void Game::calculateNewMissileLocations()
{
	
}



bool Game::checkOverlap(Sprite* missile, size_t missileLoc, Sprite* alien, size_t alienLoc)
{
	size_t x1 = missiles[missileLoc].getX();
	size_t y1 = missiles[missileLoc].getY();
	size_t x2 = aliens[alienLoc].getX();
	size_t y2 = aliens[alienLoc].getY();
	if (x1 < x2 + alien->getWidth() && x1 + missile->getWidth() > x2 &&
		y1 < y2 + alien->getHeight() && y1 + missile->getHeight() > y2)
	{
		return true;
	}
	return false;
}

Bunker* Game::getBunkers()
{
	return bunkers;
}

size_t Game::getNumAliens()
{
	return numAliens;
}

Alien* Game::getAlien(size_t loc)
{
	return &(aliens[loc]);
}

size_t Game::getPlayerX()
{
	return player->getX();
}

void Game::setPlayerX(size_t x)
{
	player->setX(x);
}

size_t Game::getPlayerY()
{
	return player->getY();
}

void Game::setPlayerY(size_t y)
{
	player->setY(y);
}
