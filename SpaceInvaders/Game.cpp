#include "Game.h"

//give create all aliens and give them initial location
void Game::initAliens()
{
	aliens = new Alien[numAliens];
	for (size_t yi = 0; yi < 5; yi++)
	{
		for (size_t xi = 0; xi < 11; xi++)
		{
			aliens[yi * 11 + xi].setX(16 * xi + 20);
			aliens[yi * 11 + xi].setY(17 * yi + 128);
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
