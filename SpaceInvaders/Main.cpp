#include<cstdio>
#include<cstdint>
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include "Buffer.h"
#include "Game.h"
#include "SpriteAnimation.h"



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

//callback function for GLFW, when GLFW has an error it will call this function
void error_callback(int error, const char* description) 
{
	fprintf(stderr, "Error: %s\n", description);
}

int main() {

	const size_t buffer_width = 224;
	const size_t buffer_height = 256;
	const size_t alien_width = 11;
	const size_t alien_height = 8;
	const size_t player_width = 11;
	const size_t player_height = 7;

	glfwSetErrorCallback(error_callback);

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
	Buffer *buffer = new Buffer(buffer_width, buffer_height);
	uint32_t standardColor =Buffer::rgb_to_uint(0,128,0);
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

	//create sprites
	Sprite *alienSprite1 = new Sprite(alien_width, alien_height);
	alienSprite1->setData(new uint8_t[alien_width * alien_height]
		{
			0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
			0,0,0,1,0,0,0,1,0,0,0, // ...@...@...
			0,0,1,1,1,1,1,1,1,0,0, // ..@@@@@@@..
			0,1,1,0,1,1,1,0,1,1,0, // .@@.@@@.@@.
			1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
			1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
			1,0,1,0,0,0,0,0,1,0,1, // @.@.....@.@
			0,0,0,1,1,0,1,1,0,0,0  // ...@@.@@...
		});

	Sprite* alienSprite2 = new Sprite(alien_width, alien_height);
	alienSprite2->setData(new uint8_t[alien_width * alien_height]
		{
			0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
			1,0,0,1,0,0,0,1,0,0,1, // @..@...@..@
			1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
			1,1,1,0,1,1,1,0,1,1,1, // @@@.@@@.@@@
			1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
			0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
			0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
			0,1,0,0,0,0,0,0,0,1,0  // .@.......@.
		}
	);


	Sprite* playerSprite = new Sprite(player_width, player_height);
	playerSprite->setData(new uint8_t[player_width * player_height]
		{
			0,0,0,0,0,1,0,0,0,0,0, // .....@.....
			0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
			0,0,0,0,1,1,1,0,0,0,0, // ....@@@....
			0,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@.
			1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
			1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
			1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
		}
	);

	//Create animations
	Sprite** alienFrames = new Sprite * [2];
	alienFrames[0] = alienSprite1;
	alienFrames[1] = alienSprite2;

	SpriteAnimation* alienAnimation = new SpriteAnimation(2, 10, 0, alienFrames);
	

	//Create game
	Game* game = new Game(buffer_width, buffer_height, 55);

	glfwSwapInterval(2);

	//game loop
	//const int target_fps = 30;
	//double lasttime = glfw()
	int playerMovementDirection = 1;
	while (!glfwWindowShouldClose(window)) 
	{
		buffer->clearBuffer(standardColor);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


		//Draw aliens
		for (size_t ai = 0; ai < game->getNumAliens(); ai++)
		{
			Alien* alien = game->getAlien(ai);
			size_t currentFrame = alienAnimation->getTime() / alienAnimation->getFrameDuration();
			Sprite* spriteToDraw = alienAnimation->getFrames()[currentFrame];
			buffer->drawSprite(*spriteToDraw, alien->getX(), alien->getY(), Buffer::rgb_to_uint(128, 0, 0));
		}


		
		//move player back and forth
		if (game->getPlayerX() + playerSprite->getWidth() + playerMovementDirection >= game->getWidth() - 1)
		{
			game->setPlayerX(game->getWidth() - playerSprite->getWidth() - playerMovementDirection - 1);
			playerMovementDirection *= -1;
		}
		else if ((int)game->getPlayerX() + playerMovementDirection <= 0)
		{
			game->setPlayerX(0);
			playerMovementDirection *= -1;
		}
		else
		{
			game->setPlayerX(game->getPlayerX() + playerMovementDirection);
		}
		buffer->drawSprite(*playerSprite, game->getPlayerX(), game->getPlayerY(), Buffer::rgb_to_uint(128, 0, 0));

		glTexSubImage2D(
			GL_TEXTURE_2D, 0, 0, 0,
			buffer->getWidth(), buffer->getHeight(),
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
			buffer->getData()
		);

		alienAnimation->addTime();
		if (alienAnimation->getTime() == alienAnimation->getNumFrames() * alienAnimation->getFrameDuration())
		{
			if (alienAnimation->isLooping())
			{
				alienAnimation->resetTime();
			}
			else
			{
				delete alienAnimation;
				alienAnimation = nullptr;
			}
		}


		glfwSwapBuffers(window);

		glfwPollEvents();
	}


	//close GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	delete alienSprite1;
	delete alienSprite2;
	delete playerSprite;
	delete buffer;
	delete game;
	return 0;

}