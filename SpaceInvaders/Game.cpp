#include "Game.h"
#include "SpriteAnimation.h"
#include <cstdlib>
#include <time.h>

using namespace sprites;

void Game::initBunkers()
{
	bunkers = new Bunker[4];
	for (int i = 0; i < 4; i++)
	{
		bunkers[i] = *new Bunker(i * 50 + 30, 30);
	}
}

//give create all aliens and give them initial location
void Game::initAliens()
{
	aliens = new Alien[numAliens];
	deathCounters = new uint8_t[numAliens];
	for (size_t yi = 0; yi < 5; yi++)
	{
		for (size_t xi = 0; xi < 11; xi++)
		{
			size_t x = 16 * xi + 20;
			size_t y = 17 * yi + 153;
			AlienType t = (AlienType)((5 - yi) / 2 + 1);
			
			aliens[yi * 11 + xi] = *new Alien(x, y, t);
			deathCounters[yi * 11 + xi] = 10;
		}
	}
}

void Game::resetAlienPositions()
{
	for (size_t yi = 0; yi < 5; yi++)
	{
		for (size_t xi = 0; xi < 11; xi++)
		{
			aliens[yi * 11 + xi].setX(16 * xi + 20);
			aliens[yi * 11 + xi].setY(17 * yi + 153);
		}
	}
}

Game::Game(size_t width, size_t height, size_t numAliens, Player* player)
{
	this->gameOver = false;
	this->continueRound = true;
	this->width = width;
	this->height = height;
	this->score = 0;
	this->numBunkers = 4;
	this->chanceToShoot = 10;
	this->alienMoveSpeed = 9;
	this->alienMoveDirection = 1;
	this->numAliens = numAliens;
	this->player = player;
	initAliens();
	initBunkers();
	srand(time(NULL));
}

Game::Game(size_t width, size_t height, size_t numAliens)
{
	this->gameOver = false;
	this->continueRound = true;
	this->width = width;
	this->height = height;
	this->score = 0;
	this->numBunkers = 4;
	this->chanceToShoot = 10;
	this->alienMoveSpeed = 9;
	this->alienMoveDirection = 1;
	this->numAliens = numAliens;
	this->player = new Player();
	initAliens();
	initBunkers();
	srand(time(NULL));
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

void Game::playerFireMissile(size_t width, size_t height, int dir)
{
	missiles[numMissiles] = *new Missile(player->getX() + width / 2, player->getY() + height, dir);
	numMissiles++;
}

void Game::alienFireMissile(Alien* alien, size_t width, size_t height, int dir)
{
	missiles[numMissiles] = *new Missile(alien->getX() + width / 2, alien->getY() + height, dir);
	numMissiles++;
}

void Game::shootForPlayer()
{
	if (numMissiles < MAX_MISSILES && getPlayerMissileCount() < PLAYER_MAX_MISSILES)
	{
		playerFireMissile(playerSprite->getWidth(), playerSprite->getHeight(), 4);

	}
	
}

void Game::shootAlienMissiles()
{
	

	for (size_t i = 0; i < numAliens; i++)
	{
		Alien alien = aliens[i];

		if (rand() % 10000 + 1 >= chanceToShoot || numMissiles == MAX_MISSILES)
			continue;
		if (alien.getType() == ALIEN_DEAD)
			continue;

		SpriteAnimation* animation = alienAnimation[alien.getType() - 1];
		size_t currentFrameOfAnimation = animation->getTime() / animation->getFrameDuration();
		Sprite* shootingSprite = animation->getFrames()[currentFrameOfAnimation];

		alienFireMissile(&alien, shootingSprite->getWidth(), shootingSprite->getHeight(), -2);

	}
}

void Game::checkCollisions()
{
	for (size_t currentMissile = 0; currentMissile <numMissiles;)
	{
		moveMissile(currentMissile);
		if (missiles[currentMissile].getY() >= height || missiles[currentMissile].getY() < 3)
		{
			removeMissile(currentMissile);
			continue;
		}

		//check missile collisions
		for (size_t currentAlien = 0; currentAlien < numAliens; currentAlien++)
		{
			Alien alien = aliens[currentAlien];
			if (alien.getType() == ALIEN_DEAD)
				continue;

			SpriteAnimation* animation = alienAnimation[alien.getType() - 1];
			size_t currentFrameOfAnimation = animation->getTime() / animation->getFrameDuration();
			Sprite* alienSprite = animation->getFrames()[currentFrameOfAnimation];

			//check if player missile hit alien
			if (missiles[currentMissile].getDir() > 0)
			{
				bool overlap = checkAlienHit(missileSprite, currentMissile, alienSprite, currentAlien);
				if (overlap)
				{
					score += 10 * (4 - aliens[currentAlien].getType());
					aliens[currentAlien].setType(ALIEN_DEAD);
					removeMissile(currentMissile);
				}

			}

		}
		//check if alien missile hit player or bunker
		if (missiles[currentMissile].getDir() < 0)
		{
			bool overlap = checkPlayerHit(missileSprite, currentMissile, playerSprite);
			if (overlap)
			{
				continueRound = false;
				player->died();
				startNewRound();
				if (player->getLives() == 0)
					gameOver = true;
			}
			else
			{
				//check bunker hit
				for (size_t currentBunker = 0; currentBunker < numBunkers; currentBunker++)
				{
					if (bunkers[currentBunker].getlife() == 0)
						continue;

					overlap = checkBunkerHit(missileSprite, currentMissile, bunkerSprites[bunkers[currentBunker].getlife() - 1], currentBunker);
					if (overlap)
					{
						bunkers[currentBunker].takeDamage();
						removeMissile(currentMissile);
					}
				}
			}

		}
		else // check if player hit bunker
		{
			for (size_t currentBunker = 0; currentBunker < numBunkers; currentBunker++)
			{
				if (bunkers[currentBunker].getlife() == 0)
					continue;

				bool overlap = checkBunkerHit(missileSprite, currentMissile, bunkerSprites[bunkers[currentBunker].getlife() - 1], currentBunker);
				if (overlap)
				{
					removeMissile(currentMissile); // just remove missile, no pushinment for hit
				}
			}
		}

		currentMissile++;
	}



	//check if alien hits bunker or player
	for (size_t currentAlien = 0; currentAlien < numAliens; currentAlien++)
	{
		Alien alien = aliens[currentAlien];
		if (alien.getType() == ALIEN_DEAD)
			continue;

		SpriteAnimation* animation = alienAnimation[alien.getType() - 1];
		size_t currentFrameOfAnimation = animation->getTime() / animation->getFrameDuration();
		Sprite* alienSprite = animation->getFrames()[currentFrameOfAnimation];

		for (size_t currentBunker = 0; currentBunker < numBunkers; currentBunker++)
		{
			if (bunkers[currentBunker].getlife() == 0)
				continue;

			bool overlap = checkAlienHitBunker(alienSprite, currentAlien, bunkerSprites[bunkers[currentBunker].getlife() - 1], currentBunker);
			if (overlap)
			{
				bunkers[currentBunker].takeDamage();
			}
		}

		bool overlap = checkAlienHitPlayer(alienSprite, currentAlien, playerSprite);
		if (overlap)
		{
			continueRound = false;
			player->died();
			startNewRound();
			if (player->getLives() == 0)
				gameOver = true;
		}
	}
}

void Game::movePlayer(int direction)
{
	playerMoveDirection = 2 * direction;
	if (playerMoveDirection != 0)
	{
		//check if player would hit right boundry
		if (player->getX() + playerSprite->getWidth() + playerMoveDirection >= width)
		{
			player->setX(width - playerSprite->getWidth());
		}
		//check if player would hit left boundry
		else if ((int)player->getX() + playerMoveDirection <= 0)
		{
			player->setX(0);
		}
		else
		{
			player->setX(player->getX() + playerMoveDirection);
		}
	}
}


void Game::moveAliens(unsigned int currentFrame)
{
	if (currentFrame % alienMoveSpeed != 0)
		return;

	for (size_t i = 0; i < numAliens; i++)
	{
		if (aliens[i].getType() == ALIEN_DEAD)
			continue;

		SpriteAnimation* animation = alienAnimation[aliens[i].getType() - 1];
		size_t currentFrameOfAnimation = animation->getTime() / animation->getFrameDuration();
		Sprite* alienSprite = animation->getFrames()[currentFrameOfAnimation];

		//check if alien hit right wall
		if (aliens[i].getX() + alienSprite->getWidth() + alienMoveDirection >= width)
		{
			//move all aliens down
			for (size_t j = 0; j < numAliens; j++)
			{
				if (aliens[i].getType() == ALIEN_DEAD)
					continue;

				aliens[j].setY(aliens[j].getY() - 5);
				if (aliens[j].getY() < player->getY())
				{
					continueRound = false;
					player->died();
					startNewRound();

					if (player->getLives() == 0)
						gameOver = true;
				}

			}
			alienMoveDirection *= -1;
			break;
		}
		else if ((int)aliens[i].getX() + alienMoveDirection <= 0)
		{
			for (size_t j = 0; j < numAliens; j++)
			{
				if (aliens[i].getType() == ALIEN_DEAD)
					continue;

				aliens[j].setY(aliens[j].getY() - 5);
			}
			alienMoveDirection *= -1;
			break;
		}
		else
		{
			aliens[i].setX(aliens[i].getX() + alienMoveDirection);
		}

	}
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



bool Game::checkAlienHit(Sprite* missile, size_t missileLoc, Sprite* alien, size_t alienLoc)
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

bool Game::checkPlayerHit(Sprite* missileSprite, size_t missileLoc, Sprite* playerSprite)
{
	size_t missileX = missiles[missileLoc].getX();
	size_t missileY = missiles[missileLoc].getY();
	size_t playerX = player->getX();
	size_t playerY = player->getY();
	if (missileX < playerX + playerSprite->getWidth() && missileX + missileSprite->getWidth() > playerX &&
		missileY < playerY + playerSprite->getHeight() && missileY + missileSprite->getHeight() > playerY)
	{
		return true;
	}
	return false;
	
}

bool Game::checkBunkerHit(Sprite* missileSprite, size_t missileLoc, Sprite* bunkerSprite, size_t bunkerLoc)
{
	size_t missileX = missiles[missileLoc].getX();
	size_t missileY = missiles[missileLoc].getY();
	size_t bunkerX = bunkers[bunkerLoc].getX();
	size_t bunkerY = bunkers[bunkerLoc].getY();
	if (missileX < bunkerX + bunkerSprite->getWidth() && missileX + missileSprite->getWidth() > bunkerX &&
		missileY < bunkerY + bunkerSprite->getHeight() && missileY + missileSprite->getHeight() > bunkerY)
	{
		return true;
	}
	return false;
}

bool Game::checkAlienHitBunker(Sprite* alienSprite, size_t alienLoc, Sprite* bunkerSprite, size_t bunkerLoc)
{
	size_t alienX = aliens[alienLoc].getX();
	size_t alienY = aliens[alienLoc].getY();
	size_t bunkerX = bunkers[bunkerLoc].getX();
	size_t bunkerY = bunkers[bunkerLoc].getY();
	if (alienX < bunkerX + bunkerSprite->getWidth() && alienX + alienSprite->getWidth() > bunkerX &&
		alienY < bunkerY + bunkerSprite->getHeight() && alienY + alienSprite->getHeight() > bunkerY)
	{
		return true;
	}
	return false;
}

bool Game::checkAlienHitPlayer(Sprite* alienSprite, size_t alienLoc, Sprite* playerSprite)
{
	size_t alienX = aliens[alienLoc].getX();
	size_t alienY = aliens[alienLoc].getY();
	size_t playerX = player->getX();
	size_t playerY = player->getY();
	if (alienX < playerX + playerSprite->getWidth() && alienX + alienSprite->getWidth() > playerX &&
		alienY < playerY + playerSprite->getHeight() && alienY + alienSprite->getHeight() > playerY)
	{
		return true;
	}
	return false;
}

void Game::drawGame(Buffer* buffer)
{
	//display score
	buffer->drawText(textSpriteSheet, "SCORE", 4, height - textSpriteSheet->getHeight() - 7, Buffer::rgb_to_uint(128, 0, 0));

	buffer->drawNumber(numberSpriteSheet, score,
		4 + 2 * numberSpriteSheet->getWidth(),
		height - 2 * numberSpriteSheet->getHeight() - 12,
		Buffer::rgb_to_uint(128, 0, 0));

	//horrizontal line at bottom 
	for (size_t i = 0; i < width; ++i)
	{
		buffer->getData()[width * 16 + i] = Buffer::rgb_to_uint(128, 0, 0);
	}


	if (gameOver)
	{
		buffer->drawText(textSpriteSheet, "GAME OVER", width / 2, height - textSpriteSheet->getHeight() - height / 2, Buffer::rgb_to_uint(128, 0, 0));
		return;
	}

	//display player lives as ship icon
	for (size_t i = 0; i < player->getLives() - 1; i++)
	{
		buffer->drawSprite(playerSprite, i * playerSprite->getWidth() + 10, 5, Buffer::rgb_to_uint(128, 0, 0));
	}

	//Draw bunkers
	for (size_t i = 0; i < 4; i++)
	{
		Bunker bunker = bunkers[i];
		if (bunker.getlife() == 0)
			continue;

		buffer->drawSprite(bunkerSprites[bunker.getlife() - 1], bunker.getX(), bunker.getY(), Buffer::rgb_to_uint(128, 0, 0));

	}

	//Draw aliens
	numDeadAliens = 0;
	for (size_t i = 0; i < numAliens; i++)
	{
		if (!deathCounters[i])
		{
			numDeadAliens++;
			continue;
		}


		Alien* alien = &aliens[i];
		if (alien->getType() == ALIEN_DEAD)
		{
			buffer->drawSprite(deadAlienSprite, alien->getX(), alien->getY(), Buffer::rgb_to_uint(128, 0, 0));
		}
		else
		{
			SpriteAnimation* animation = alienAnimation[alien->getType() - 1];
			size_t currentFrameOfAnimation = animation->getTime() / animation->getFrameDuration();
			Sprite* spriteToDraw = animation->getFrames()[currentFrameOfAnimation];
			buffer->drawSprite(spriteToDraw, alien->getX(), alien->getY(), Buffer::rgb_to_uint(128, 0, 0));
		}
	}

	//update death counters for aliens
	for (size_t i = 0; i < numAliens; i++)
	{
		Alien alien = aliens[i];
		if (alien.getType() == ALIEN_DEAD && deathCounters[i])
		{
			deathCounters[i]--;
		}
	}

	//Draw missiles
	for (size_t i = 0; i < numMissiles; i++)
	{
		Missile missile = missiles[i];
		Sprite* spriteToDraw = missileSprite;
		buffer->drawSprite(missileSprite, missile.getX(), missile.getY(), Buffer::rgb_to_uint(128, 0, 0));
	}

	//update time for sprites animations
		for (size_t i = 0; i < 3; i++)
		{
			alienAnimation[i]->addTime();
			if (alienAnimation[i]->getTime() == alienAnimation[i]->getNumFrames() * alienAnimation[i]->getFrameDuration())
			{
				if (alienAnimation[i]->isLooping())
				{
					alienAnimation[i]->resetTime();
				}
			}
		}

	buffer->drawSprite(playerSprite, player->getX(), player->getY(), Buffer::rgb_to_uint(128, 0, 0));


	switch (numDeadAliens)
	{
	case 0:
		alienMoveSpeed = 9;
		chanceToShoot = 3;
		break;
	case 6:
		alienMoveSpeed = 8;
		chanceToShoot = 7;
	case 11:
		alienMoveSpeed = 7;
		chanceToShoot = 10;
		break;
	case 15:
		alienMoveSpeed = 6;
		chanceToShoot = 13;
		break;
	case 22:
		alienMoveSpeed = 5;
		chanceToShoot = 16;
		break;
	case 26:
		alienMoveSpeed = 4;
		chanceToShoot = 23;
		break;
	case 33:
		alienMoveSpeed = 3;
		chanceToShoot = 30;
		break;
	case 37:
		chanceToShoot = 40;
		break;
	case 44:
		alienMoveSpeed = 2;
		chanceToShoot = 50;
		break;
	case 54:
		alienMoveSpeed = 1;
		chanceToShoot = 100;
		break;
	case 55:
		startNewRound();
		continueRound = false;
		break;
	}
}



void Game::startNewRound()
{
	numMissiles = 0;
	player->setX(107);
	size_t deadAliens = 0;
	for (size_t i = 0; i < numAliens; i++)
	{
		if(aliens[i].getType() == ALIEN_DEAD)
			deadAliens++;
	}
	if (deadAliens == numAliens)
		initAliens();
	else
		resetAlienPositions();

	initBunkers();
}


size_t Game::getPlayerMissileCount()
{
	size_t playerMissiles = 0;
	for (size_t i = 0; i < numMissiles; i++)
	{
		if (missiles[i].getDir() > 0)
			playerMissiles++;
	}
	return playerMissiles;
}


bool Game::isGameOver()
{
	return gameOver;
}

bool Game::shouldContinueRound()
{
	return continueRound;
}

void Game::setContinueRound(bool continueRound)
{
	this->continueRound = continueRound;
}
