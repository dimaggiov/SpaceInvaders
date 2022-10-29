#include<cstdio>
#include<cstdint>
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include "Buffer.h"
#include "Game.h"
#include <cstdlib>
#include <time.h>


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

	

	//Create game
	Game* game = new Game(buffer_width, buffer_height, 55);

	glfwSwapInterval(2);
	
	srand(time(NULL));

	unsigned int currentFrame = 0; //for moving aliens, dont care if it overflows and resets to 0
	size_t countDeadAliens = 0;
	int chanceToShoot = 10;

	//game loop
	while (!glfwWindowShouldClose(window) && !quitPressed) 
	{
		buffer->clearBuffer(standardColor);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		currentFrame++;

		gameOver = game->isGameOver();

		if (resetGame)
		{
			game = new Game(buffer_width, buffer_height, 55);
			resetGame = false;
		}


		game->drawGame(buffer);
		

		glfwSwapBuffers(window);
		glfwPollEvents();
		glTexSubImage2D(
			GL_TEXTURE_2D, 0, 0, 0,
			buffer->getWidth(), buffer->getHeight(),
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
			buffer->getData()
		);

		game->setContinueRound(gameStarted);

   		if (!game->shouldContinueRound())
		{
			gameStarted = false;
			continue;
		}
		

		game->shootAlienMissiles();

		game->checkCollisions();
		gameStarted = game->shouldContinueRound();

		game->movePlayer(movementDirection);
		

		if (firePressed)
		{
			game->shootForPlayer();
		}
		firePressed = false;

		game->moveAliens(currentFrame);
		

	}


	//close GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	delete buffer;
	delete game;
	return 0;
	
}
