#include "Game.h"

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
	this->numAliens = numAliens;
	this->player = player;
	initAliens();
}

Game::Game(size_t width, size_t height, size_t numAliens)
{
	this->width = width;
	this->height = height;
	this->numAliens = numAliens;
	this->player = new Player();
	initAliens();
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

size_t Game::getNumMissiles()
{
	return numMissiles;
}

void Game::fireMissile(size_t width, size_t height, int dir)
{
	missiles[numMissiles] = *new Missile(player->getX() + width / 2, player->getY() + height, dir);
	numMissiles++;
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
	for (size_t i = 0; i < numMissiles;)
	{
		moveMissile(i);
		if (missiles[i].getY() >= height || missiles[i].getY() < 3)
		{
			missiles[i] = missiles[numMissiles - 1];
			numMissiles--;
			continue;
		}
		i++;
	}
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
