#include<cstdio>
#include<cstdint>
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include "Buffer.h"
#include "Game.h"
#include "SpriteAnimation.h"
#include <cstdlib>
#include <time.h>
#include "Sprites.h"

using namespace sprites;

typedef void(*GLFWerrorfun)(int, const char*);
typedef void(*GLFWkeyfun)(GLFWwindow*, int, int, int, int);

//when GLFW has an error it will call this function
void errorCallback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}


bool quitPressed = false;
int movementDirection = 0;// 0 is no movement, -1 is left, 1 is right
bool firePressed = false;
bool gameStarted = false;
bool gameOver = false;
bool resetGame = false;
int alienMovementDirection = -1;
int alienMoveSpeed = 8;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		if (action == GLFW_PRESS)
			quitPressed = true;
		break;

	//Movement
	//this creates a bug that when pressing both buttons you move 2x as fast but I like it so I'm leaving it in
	case GLFW_KEY_A:
	case GLFW_KEY_LEFT:
 		if (action == GLFW_PRESS)
			movementDirection -= 1;
		else if (action == GLFW_RELEASE)
			movementDirection += 1;
		break;
	case GLFW_KEY_D:
	case GLFW_KEY_RIGHT:
		if (action == GLFW_PRESS)
			movementDirection += 1;
		else if (action == GLFW_RELEASE)
			movementDirection -= 1;
		break;

	//Firing missile or start game
	case GLFW_KEY_SPACE:
		if (action == GLFW_PRESS)
		{

			if (gameOver)
			{
				gameOver = false;
				resetGame = true;
				break;
			}
			if (!gameStarted)
			{
 				gameStarted = true;
				break;
			}
			firePressed = true;
		}
		break;

	default:
		break;
	}
}

void validateShader(GLuint shader, const char* file = 0)
{
	static const unsigned int BUFFER_SIZE = 512;
	char buffer[BUFFER_SIZE];
	GLsizei length = 0;

	glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);

	if (length > 0)
	{
		printf("Shader %d(%s) compile error: %s\n", shader, (file ? file : ""), buffer);
	}
}

bool validateProgram(GLuint program)
{
	static const unsigned int BUFFER_SIZE = 512;
	GLchar buffer[BUFFER_SIZE];
	GLsizei length = 0;

	glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);

	if (length > 0)
	{
		printf("Program %d link error: %s\n", program, buffer);
		return false;
	}
	return true;
}


int main() {

	const size_t buffer_width = 224;
	const size_t buffer_height = 256;
	const size_t player_width = 11;
	const size_t player_height = 7;
	const size_t missile_width = 1;
	const size_t missile_height = 3;

	glfwSetErrorCallback(errorCallback);

	//initilize GLFW
	if (!glfwInit())
		return -1;

	GLFWwindow* window = glfwCreateWindow(640, 480, "Space Invaders", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCallback);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//initilize GLEW
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "Error initializing GLEW.\n");
		glfwTerminate();
		return -1;
	}

	int glVersion[2] = { -1,1 };
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

	printf("Using OpenGL: %d.%d\n", glVersion[0], glVersion[1]);

	glClearColor(1.0, 0.0, 0.0, 1.0);


	//create buffer for game
	Buffer* buffer = new Buffer(buffer_width, buffer_height);
	uint32_t standardColor = Buffer::rgb_to_uint(0, 128, 0);
	buffer->setData(new uint32_t[buffer->getWidth() * buffer->getHeight()]);
	buffer->clearBuffer(standardColor);




	//create buffer texture
	GLuint bufferTexture;
	glGenTextures(1, &bufferTexture);
	glBindTexture(GL_TEXTURE_2D, bufferTexture);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGB8,
		buffer->getWidth(), buffer->getHeight(), 0,
		GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer->getData()
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLuint fullScreenVAO;
	glGenVertexArrays(1, &fullScreenVAO);

	//create shaders
	static const char* vertexShader =
		"\n"
		"#version 330\n"
		"\n"
		"noperspective out vec2 TexCoord;\n"
		"\n"
		"void main(void){\n"
		"\n"
		"    TexCoord.x = (gl_VertexID == 2)? 2.0: 0.0;\n"
		"    TexCoord.y = (gl_VertexID == 1)? 2.0: 0.0;\n"
		"    \n"
		"    gl_Position = vec4(2.0 * TexCoord - 1.0, 0.0, 1.0);\n"
		"}\n";

	static const char* fragmentShader =
		"\n"
		"#version 330\n"
		"\n"
		"uniform sampler2D buffer;\n"
		"noperspective in vec2 TexCoord;\n"
		"\n"
		"out vec3 outColor;\n"
		"\n"
		"void main(void){\n"
		"    outColor = texture(buffer, TexCoord).rgb;\n"
		"}\n";



	GLuint shaderID = glCreateProgram();

	//create vertex shader
	GLuint vpShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vpShader, 1, &vertexShader, 0);
	glCompileShader(vpShader);
	validateShader(vpShader, vertexShader);
	glAttachShader(shaderID, vpShader);
	glDeleteShader(vpShader);

	//create fragment shader
	GLuint fpShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fpShader, 1, &fragmentShader, 0);
	glCompileShader(fpShader);
	validateShader(fpShader, fragmentShader);
	glAttachShader(shaderID, fpShader);
	glDeleteShader(fpShader);

	//validate shaders
	glLinkProgram(shaderID);

	if (!validateProgram(shaderID))
	{
		fprintf(stderr, "Error while validating shaders.\n");
		glfwTerminate();
		glDeleteVertexArrays(1, &fullScreenVAO);
		delete buffer;
		return -1;
	}

	glUseProgram(shaderID);

	GLuint location = glGetUniformLocation(shaderID, "buffer");
	glUniform1i(location, 0);

	//setup OpenGL
	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);

	glBindVertexArray(fullScreenVAO);


	
	//Create an animation for each of the aliens
	SpriteAnimation alienAnimation[3] = 
	{ 
		*new SpriteAnimation(2, 10, 0, new Sprite* [2]
			{
				alienSprites[0], alienSprites[1]
			}),
		*new SpriteAnimation(2, 10, 0, new Sprite* [2]
			{
				alienSprites[2], alienSprites[3]
			}),
		*new SpriteAnimation(2, 10, 0, new Sprite* [2]
			{
				alienSprites[4], alienSprites[5]
			})
	};
	

	//Create game
	Game* game = new Game(buffer_width, buffer_height, 55);

	uint8_t* deathCounters = new uint8_t[game->getNumAliens()];
	for (size_t i = 0; i < game->getNumAliens(); i++)
	{
		deathCounters[i] = 10;
	}

	glfwSwapInterval(2);
	
	srand(time(NULL));

	unsigned int currentFrame = 0; //for moving aliens, dont care if it overflows and resets to 0
	size_t countDeadAliens = 0;
	int chanceToShoot = 10;

	//game loop
	while (!glfwWindowShouldClose(window) && !quitPressed) 
	{
		
		currentFrame++;

		if (resetGame)
		{
			game = new Game(buffer_width, buffer_height, 55);
			resetGame = false;
			for (size_t i = 0; i < game->getNumAliens(); i++)
			{
				deathCounters[i] = 10;
			}
		}

		buffer->clearBuffer(standardColor);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		//display score
		buffer->drawText(textSpriteSheet, "SCORE", 4, game->getHeight() - textSpriteSheet->getHeight() - 7, Buffer::rgb_to_uint(128,0,0));

		buffer->drawNumber(numberSpriteSheet, game->getScore(),
			4 + 2 * numberSpriteSheet->getWidth(),
			game->getHeight() - 2 * numberSpriteSheet->getHeight() - 12,
			Buffer::rgb_to_uint(128, 0, 0));

		//horrizontal line at bottom 
		for (size_t i = 0; i < game->getWidth(); ++i)
		{
			buffer->getData()[game->getWidth() * 16 + i] = Buffer::rgb_to_uint(128, 0, 0);
		}


		if (gameOver)
		{
			buffer->drawText(textSpriteSheet, "GAME OVER", game->getWidth() / 2, game->getHeight() - textSpriteSheet->getHeight() - game->getHeight() / 2, Buffer::rgb_to_uint(128, 0, 0));
			glfwSwapBuffers(window);

			glfwPollEvents();

			glTexSubImage2D(
				GL_TEXTURE_2D, 0, 0, 0,
				buffer->getWidth(), buffer->getHeight(),
				GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
				buffer->getData()
			);
			continue;
		}

		//display player lives as ship icon
		for (size_t i = 0; i < game->getPlayerLives() - 1; i++)
		{
			buffer->drawSprite(playerSprite, i * playerSprite->getWidth() + 10, 5, Buffer::rgb_to_uint(128, 0, 0));
		}

		

		//Draw bunkers
		for (size_t i = 0; i < 4; i++)
		{
			Bunker bunker = game->getBunkers()[i];
			if (bunker.getlife() == 0)
				continue;

			buffer->drawSprite(bunkerSprites[bunker.getlife() - 1], bunker.getX(), bunker.getY(), Buffer::rgb_to_uint(128, 0, 0));
		
		}


		//Draw aliens
		countDeadAliens = 0;
		for (size_t i = 0; i < game->getNumAliens(); i++)
		{
			if (!deathCounters[i])
			{
				countDeadAliens++;
				continue;
			}


			Alien* alien = game->getAlien(i);
			if (alien->getType() == ALIEN_DEAD)
			{
				buffer->drawSprite(deadAlienSprite, alien->getX(), alien->getY(), Buffer::rgb_to_uint(128, 0, 0));
			}
			else
			{
				SpriteAnimation animation = alienAnimation[alien->getType()-1];
				size_t currentFrameOfAnimation = animation.getTime() / animation.getFrameDuration();
				Sprite* spriteToDraw = animation.getFrames()[currentFrameOfAnimation];
				buffer->drawSprite(spriteToDraw, alien->getX(), alien->getY(), Buffer::rgb_to_uint(128, 0, 0));
			}
		}

		//update alien speed based on number of dead aliens
		//or end round if all dead
		switch (countDeadAliens)
		{
		case 0:
			alienMoveSpeed = 9;
			chanceToShoot = 10;
			break;
		case 6:
			alienMoveSpeed = 8;
			chanceToShoot = 12;
		case 11:
			alienMoveSpeed = 7;
			chanceToShoot = 13;
			break;
		case 15:
			alienMoveSpeed = 6;
			chanceToShoot = 15;
			break;
		case 22: 
			alienMoveSpeed = 5;
			chanceToShoot = 20;
			break;
		case 26:
			alienMoveSpeed = 4;
			chanceToShoot = 25;
			break;
		case 33:
			alienMoveSpeed = 3;
			chanceToShoot = 30;
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
			game->startNewRound();
			for (size_t i = 0; i < game->getNumAliens(); i++)
			{
				deathCounters[i] = 10;
			}
			gameStarted = false;
			break;
		}




		//Draw missiles
		for (size_t i = 0; i < game->getNumMissiles(); i++)
		{
			Missile missile = game->getMissiles()[i];
			Sprite* spriteToDraw = missileSprite;
			buffer->drawSprite(missileSprite, missile.getX(), missile.getY(), Buffer::rgb_to_uint(128, 0, 0));
		}

		//update time for sprites animations
		for (size_t i = 0; i < 3; i++)
		{
			alienAnimation[i].addTime();
			if (alienAnimation[i].getTime() == alienAnimation[i].getNumFrames() * alienAnimation[i].getFrameDuration())
			{
				if (alienAnimation[i].isLooping())
				{
					alienAnimation[i].resetTime();
				}
			}
		}

		buffer->drawSprite(playerSprite, game->getPlayerX(), game->getPlayerY(), Buffer::rgb_to_uint(128, 0, 0));

		glfwSwapBuffers(window);

		glfwPollEvents();

		glTexSubImage2D(
			GL_TEXTURE_2D, 0, 0, 0,
			buffer->getWidth(), buffer->getHeight(),
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
			buffer->getData()
		);


		//gameplay code goes below here, only happens if game is started
		if (!gameStarted)
			continue;


		//shoot missile from alien
		for (size_t i = 0; i < game->getNumAliens(); i++)
		{
			Alien* alien = game->getAlien(i);

			if (rand() % 10000 + 1 >= chanceToShoot || game->getNumMissiles() == Game::MAX_MISSILES) //.1% chance for every alien to shoot
				continue;
			if (alien->getType() == ALIEN_DEAD)
				continue;

			SpriteAnimation animation = alienAnimation[alien->getType() - 1];
			size_t currentFrameOfAnimation = animation.getTime() / animation.getFrameDuration();
			Sprite* shootingSprite = animation.getFrames()[currentFrameOfAnimation];

			game->alienFireMissile(alien, shootingSprite->getWidth(), shootingSprite->getHeight(), -2);

		}

		//update death counters for aliens
		for (size_t i = 0; i < game->getNumAliens(); i++)
		{
			Alien* alien = game->getAlien(i);
			if (alien->getType() == ALIEN_DEAD && deathCounters[i])
			{
				deathCounters[i]--;
			}
		}

		//Calculate new missile locations
		for (size_t currentMissile = 0; currentMissile < game->getNumMissiles();)
		{
			game->moveMissile(currentMissile);
			if (game->getMissiles()[currentMissile].getY() >= game->getHeight() || game->getMissiles()[currentMissile].getY() < 3)
			{
				game->removeMissile(currentMissile);
				continue;
			}


			//check missile collisions
			for (size_t currentAlien = 0; currentAlien < game->getNumAliens(); currentAlien++)
			{
				Alien* alien = game->getAlien(currentAlien);
				if (alien->getType() == ALIEN_DEAD)
					continue;

				SpriteAnimation animation = alienAnimation[alien->getType() - 1];
				size_t currentFrameOfAnimation = animation.getTime() / animation.getFrameDuration();
				Sprite* alienSprite = animation.getFrames()[currentFrameOfAnimation];

				//check if player missile hit alien
				if (game->getMissiles()[currentMissile].getDir() > 0)
				{
					bool overlap = game->checkAlienHit(missileSprite, currentMissile, alienSprite, currentAlien);
					if (overlap)
					{
						game->addScore(10 * (4 - game->getAlien(currentAlien)->getType()));
						game->getAlien(currentAlien)->setType(ALIEN_DEAD);
						game->removeMissile(currentMissile);
					}
				
				}

			}
			//check if alien missile hit player or bunker
			if (game->getMissiles()[currentMissile].getDir() < 0)
			{
				bool overlap = game->checkPlayerHit(missileSprite, currentMissile, playerSprite);
				if (overlap)
				{
					gameStarted = false;
					game->playerHit();
					if (game->getPlayerLives() == 0)
						gameOver = true;
				}
				else
				{
					//check bunker hit
					for (size_t currentBunker = 0; currentBunker < game->getNumBunkers(); currentBunker++)
					{
						if (game->getBunkers()[currentBunker].getlife() == 0)
							continue;
						
						overlap = game->checkBunkerHit(missileSprite, currentMissile, bunkerSprites[game->getBunkers()[currentBunker].getlife() -1], currentBunker);
						if (overlap)
						{
							game->bunkerHit(currentBunker);
							game->removeMissile(currentMissile);
						}
					}
				}
			
			}
			else // check if player hit bunker
			{
				for (size_t currentBunker = 0; currentBunker < game->getNumBunkers(); currentBunker++)
				{
					if (game->getBunkers()[currentBunker].getlife() == 0)
						continue;

					bool overlap = game->checkBunkerHit(missileSprite, currentMissile, bunkerSprites[game->getBunkers()[currentBunker].getlife() - 1], currentBunker);
					if (overlap)
					{
						game->removeMissile(currentMissile); // just remove missile, no pushinment for hit
					}
				}
			}
			
			currentMissile++;
		}



		//check if alien hits bunker or player
		for (size_t currentAlien = 0; currentAlien < game->getNumAliens(); currentAlien++)
		{
			Alien* alien = game->getAlien(currentAlien);
			if (alien->getType() == ALIEN_DEAD)
				continue;

			SpriteAnimation animation = alienAnimation[alien->getType() - 1];
			size_t currentFrameOfAnimation = animation.getTime() / animation.getFrameDuration();
			Sprite* alienSprite = animation.getFrames()[currentFrameOfAnimation];

			for (size_t currentBunker = 0; currentBunker < game->getNumBunkers(); currentBunker++)
			{
				if (game->getBunkers()[currentBunker].getlife() == 0)
					continue;

				bool overlap = game->checkAlienHitBunker(alienSprite, currentAlien, bunkerSprites[game->getBunkers()[currentBunker].getlife() - 1], currentBunker);
				if (overlap)
				{
					game->getBunkers()[currentBunker].takeDamage();
				}
			}

			bool overlap = game->checkAlienHitPlayer(alienSprite, currentAlien, playerSprite);
			if (overlap)
			{
				gameStarted = false;
				game->playerHit();
				if (game->getPlayerLives() == 0)
					gameOver = true;
			}
		}
			
		



		int playerMovementDirection = 2 * movementDirection;
		if (playerMovementDirection != 0)
		{
			//check if player would hit right boundry
			if (game->getPlayerX() + playerSprite->getWidth() + playerMovementDirection >= game->getWidth())
			{
				game->setPlayerX(game->getWidth() - playerSprite->getWidth());
			}
			//check if player would hit left boundry
			else if ((int)game->getPlayerX() + playerMovementDirection <= 0)
			{
				game->setPlayerX(0);
			}
			else
			{
				game->setPlayerX(game->getPlayerX() + playerMovementDirection);
			}
		}

		
	
		//create new missle for player
		if (firePressed && game->getNumMissiles() < game->MAX_MISSILES && game->getPlayerMissileCount() < game->PLAYER_MAX_MISSILES)
		{
			game->playerFireMissile(playerSprite->getWidth(), playerSprite->getHeight(), 4);
			
		}
		firePressed = false;
		

		if (currentFrame % alienMoveSpeed != 0)
			continue;

		//move aliens
		for (size_t i = 0; i < game->getNumAliens(); i++)
		{
			Alien* alien = game->getAlien(i);
			if (alien->getType() == ALIEN_DEAD)
				continue;

			SpriteAnimation animation = alienAnimation[alien->getType() - 1];
			size_t currentFrameOfAnimation = animation.getTime() / animation.getFrameDuration();
			Sprite* alienSprite = animation.getFrames()[currentFrameOfAnimation];

			//check if alien hit right wall
			if (alien->getX() + alienSprite->getWidth() + alienMovementDirection >= game->getWidth())
			{
				//move all aliens down
				for (size_t j = 0; j < game->getNumAliens(); j++)
				{
					Alien* alienToMoveDown = game->getAlien(j);
					if (alien->getType() == ALIEN_DEAD)
						continue;

					alienToMoveDown->setY(alienToMoveDown->getY() - 5);
					if (alienToMoveDown->getY() < game->getPlayerY())
					{
						gameStarted = false;
						game->playerHit();

						if (game->getPlayerLives() == 0)
							gameOver = true;
					}

				}
				alienMovementDirection *= -1;
				break;
			}
			else if ((int)game->getAlien(i)->getX() + alienMovementDirection <= 0)
			{
				for (size_t j = 0; j < game->getNumAliens(); j++)
				{
					Alien* alienToMoveDown = game->getAlien(j);
					if (alien->getType() == ALIEN_DEAD)
						continue;

					alienToMoveDown->setY(alienToMoveDown->getY() - 5);
				}
				alienMovementDirection *= -1;
				break;
			}
			else
			{
				alien->setX(alien->getX() + alienMovementDirection);
			}

		}

		

	}


	//close GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	
	delete[] deathCounters;
	delete playerSprite;
	delete buffer;
	delete game;
	return 0;
	
}
