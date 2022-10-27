#include<cstdio>
#include<cstdint>
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include "Buffer.h"
#include "Game.h"
#include "SpriteAnimation.h"


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

	//Firing missile
	case GLFW_KEY_SPACE:
		if (action == GLFW_PRESS)
			firePressed = true;
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
	Sprite alienSprites[6];
	alienSprites[0] = *new Sprite(8, 8);
	alienSprites[0].setData(new uint8_t[64]
		{
			0,0,0,1,1,0,0,0, // ...@@...
			0,0,1,1,1,1,0,0, // ..@@@@..
			0,1,1,1,1,1,1,0, // .@@@@@@.
			1,1,0,1,1,0,1,1, // @@.@@.@@
			1,1,1,1,1,1,1,1, // @@@@@@@@
			0,1,0,1,1,0,1,0, // .@.@@.@.
			1,0,0,0,0,0,0,1, // @......@
			0,1,0,0,0,0,1,0  // .@....@.
		});

	alienSprites[1] = *new Sprite(8, 8);
	alienSprites[1].setData(new uint8_t[64]
		{
			0,0,0,1,1,0,0,0, // ...@@...
			0,0,1,1,1,1,0,0, // ..@@@@..
			0,1,1,1,1,1,1,0, // .@@@@@@.
			1,1,0,1,1,0,1,1, // @@.@@.@@
			1,1,1,1,1,1,1,1, // @@@@@@@@
			0,0,1,0,0,1,0,0, // ..@..@..
			0,1,0,1,1,0,1,0, // .@.@@.@.
			1,0,1,0,0,1,0,1  // @.@..@.@
		}
	);

	alienSprites[2] = *new Sprite(11, 8);
	alienSprites[2].setData(new uint8_t[88]
		{
			0,0,1,0,0,0,0,0,1,0,0, // ..@.....@..
			0,0,0,1,0,0,0,1,0,0,0, // ...@...@...
			0,0,1,1,1,1,1,1,1,0,0, // ..@@@@@@@..
			0,1,1,0,1,1,1,0,1,1,0, // .@@.@@@.@@.
			1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@
			1,0,1,1,1,1,1,1,1,0,1, // @.@@@@@@@.@
			1,0,1,0,0,0,0,0,1,0,1, // @.@.....@.@
			0,0,0,1,1,0,1,1,0,0,0  // ...@@.@@...
		}
	);

	alienSprites[3] = *new Sprite(11, 8);
	alienSprites[3].setData(new uint8_t[88]
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

	alienSprites[4] = *new Sprite(12, 8);
	alienSprites[4].setData(new uint8_t[96]
		{
			0,0,0,0,1,1,1,1,0,0,0,0, // ....@@@@....
			0,1,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@@.
			1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
			1,1,1,0,0,1,1,0,0,1,1,1, // @@@..@@..@@@
			1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
			0,0,0,1,1,0,0,1,1,0,0,0, // ...@@..@@...
			0,0,1,1,0,1,1,0,1,1,0,0, // ..@@.@@.@@..
			1,1,0,0,0,0,0,0,0,0,1,1  // @@........@@
		}
	);

	alienSprites[5] = *new Sprite(12, 8);
	alienSprites[5].setData(new uint8_t[96]
		{
			0,0,0,0,1,1,1,1,0,0,0,0, // ....@@@@....
			0,1,1,1,1,1,1,1,1,1,1,0, // .@@@@@@@@@@.
			1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
			1,1,1,0,0,1,1,0,0,1,1,1, // @@@..@@..@@@
			1,1,1,1,1,1,1,1,1,1,1,1, // @@@@@@@@@@@@
			0,0,1,1,1,0,0,1,1,1,0,0, // ..@@@..@@@..
			0,1,1,0,0,1,1,0,0,1,1,0, // .@@..@@..@@.
			0,0,1,1,0,0,0,0,1,1,0,0  // ..@@....@@..
		}
	);

	Sprite deadAlienSprite = *new Sprite(13, 7);
	deadAlienSprite.setData(new uint8_t[96]
		{
			0,1,0,0,1,0,0,0,1,0,0,1,0, // .@..@...@..@.
			0,0,1,0,0,1,0,1,0,0,1,0,0, // ..@..@.@..@..
			0,0,0,1,0,0,0,0,0,1,0,0,0, // ...@.....@...
			1,1,0,0,0,0,0,0,0,0,0,1,1, // @@.........@@
			0,0,0,1,0,0,0,0,0,1,0,0,0, // ...@.....@...
			0,0,1,0,0,1,0,1,0,0,1,0,0, // ..@..@.@..@..
			0,1,0,0,1,0,0,0,1,0,0,1,0  // .@..@...@..@.
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

	Sprite* missileSprite = new Sprite(missile_width, missile_height);
	missileSprite->setData(new uint8_t[missile_width * missile_height]
		{
			1,
			1,
			1,
		});

	//Text spritesheet
	Sprite textSpriteSheet = *new Sprite(5, 7);
	textSpriteSheet.setData(new uint8_t[65 * 35]
		{
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,
			0,1,0,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,1,0,1,0,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,0,0,1,0,1,0,
			0,0,1,0,0,0,1,1,1,0,1,0,1,0,0,0,1,1,1,0,0,0,1,0,1,0,1,1,1,0,0,0,1,0,0,
			1,1,0,1,0,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1,1,0,1,0,1,1,
			0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,0,1,0,1,1,1,1,
			0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,
			1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,
			0,0,1,0,0,1,0,1,0,1,0,1,1,1,0,0,0,1,0,0,0,1,1,1,0,1,0,1,0,1,0,0,1,0,0,
			0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
			0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,

			0,1,1,1,0,1,0,0,0,1,1,0,0,1,1,1,0,1,0,1,1,1,0,0,1,1,0,0,0,1,0,1,1,1,0,
			0,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,0,
			0,1,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,1,1,0,0,1,0,0,0,1,0,0,0,0,1,1,1,1,1,
			1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
			0,0,0,1,0,0,0,1,1,0,0,1,0,1,0,1,0,0,1,0,1,1,1,1,1,0,0,0,1,0,0,0,0,1,0,
			1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
			0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
			1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,
			0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
			0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,1,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,

			0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,
			0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,
			0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
			1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,
			0,1,1,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,
			0,1,1,1,0,1,0,0,0,1,1,0,1,0,1,1,1,0,1,1,1,0,1,0,0,1,0,0,0,1,0,1,1,1,0,

			0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,1,
			1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,
			0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1,1,1,0,
			1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,
			1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,1,1,1,1,1,
			1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,
			0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,1,0,1,1,1,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
			1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,
			0,1,1,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,0,
			0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
			1,0,0,0,1,1,0,0,1,0,1,0,1,0,0,1,1,0,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,1,
			1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,1,1,1,
			1,0,0,0,1,1,1,0,1,1,1,0,1,0,1,1,0,1,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,
			1,0,0,0,1,1,0,0,0,1,1,1,0,0,1,1,0,1,0,1,1,0,0,1,1,1,0,0,0,1,1,0,0,0,1,
			0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
			1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,
			0,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,1,0,1,1,0,0,1,1,0,1,1,1,1,
			1,1,1,1,0,1,0,0,0,1,1,0,0,0,1,1,1,1,1,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,1,
			0,1,1,1,0,1,0,0,0,1,1,0,0,0,0,0,1,1,1,0,1,0,0,0,1,0,0,0,0,1,0,1,1,1,0,
			1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,
			1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0,
			1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,
			1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,0,1,0,1,1,0,1,0,1,1,1,0,1,1,1,0,0,0,1,
			1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,1,0,0,0,1,
			1,0,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,
			1,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,1,1,1,1,

			0,0,0,1,1,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,1,
			0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,
			1,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,1,1,0,0,0,
			0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
			0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		}
	);

	Sprite numberSpriteSheet = textSpriteSheet;
	numberSpriteSheet.setData(numberSpriteSheet.getData() + 16 * 35);

	
	//Create an animation for each of the aliens
	SpriteAnimation alienAnimation[3] = 
	{ 
		*new SpriteAnimation(2, 10, 0, new Sprite* [2]
			{
				&alienSprites[0], &alienSprites[1]
			}),
		*new SpriteAnimation(2, 10, 0, new Sprite* [2]
			{
				&alienSprites[2],& alienSprites[3]
			}),
		*new SpriteAnimation(2, 10, 0, new Sprite* [2]
			{
				&alienSprites[4],& alienSprites[5]
			})
	};
	

	//Create game
	Game* game = new Game(buffer_width, buffer_height, 55);

	uint8_t* deathCounters = new uint8_t[game->getNumAliens()];
	for (size_t i = 0; i < game->getNumAliens(); i++)
	{
		deathCounters[i] = 10;
	}

	glfwSwapInterval(4);

	int alienMovementDirection = -1;

	//game loop
	while (!glfwWindowShouldClose(window) && !quitPressed) 
	{
		buffer->clearBuffer(standardColor);
		buffer->drawText(textSpriteSheet, "SCORE", 4, game->getHeight() - textSpriteSheet.getHeight() - 7, Buffer::rgb_to_uint(128,0,0));
		buffer->drawNumber(numberSpriteSheet, game->getScore(),
			4 + 2 * numberSpriteSheet.getWidth(), 
			game->getHeight() - 2 * numberSpriteSheet.getHeight() - 12, 
			Buffer::rgb_to_uint(128, 0, 0));


		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


		//Draw aliens
		for (size_t i = 0; i < game->getNumAliens(); i++)
		{
			if (!deathCounters[i])
				continue;


			Alien* alien = game->getAlien(i);
			if (alien->getType() == ALIEN_DEAD)
			{
				buffer->drawSprite(deadAlienSprite, alien->getX(), alien->getY(), Buffer::rgb_to_uint(128, 0, 0));
			}
			else
			{
				SpriteAnimation animation = alienAnimation[alien->getType()-1];
				if (alien->getType() == ALIEN_TYPE_B)
					int i = 0;
				size_t currentFrame = animation.getTime() / animation.getFrameDuration();
				Sprite* spriteToDraw = animation.getFrames()[currentFrame];
				buffer->drawSprite(*spriteToDraw, alien->getX(), alien->getY(), Buffer::rgb_to_uint(128, 0, 0));
			}
		}

		//Draw missiles
		for (size_t i = 0; i < game->getNumMissiles(); i++)
		{
			Missile missile = game->getMissiles()[i];
			Sprite* spriteToDraw = missileSprite;
			buffer->drawSprite(*missileSprite, missile.getX(), missile.getY(), Buffer::rgb_to_uint(128, 0, 0));
		}

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


			//check collisions
			for (size_t currentAlien = 0; currentAlien < game->getNumAliens(); currentAlien++)
			{
				Alien* alien = game->getAlien(currentAlien);
				if (alien->getType() == ALIEN_DEAD)
					continue;

				SpriteAnimation animation = alienAnimation[alien->getType() - 1];
				size_t currentFrame = animation.getTime() / animation.getFrameDuration();
				Sprite* alienSprite = animation.getFrames()[currentFrame];

				bool overlap = game->checkOverlap(missileSprite, currentMissile, alienSprite, currentAlien);
				if (overlap)
				{
					game->addScore(10 * (4 - game->getAlien(currentAlien)->getType()));
 					game->getAlien(currentAlien)->setType(ALIEN_DEAD);
					game->removeMissile(currentMissile);
				}
			}

			
			currentMissile++;
		}

		//move aliens
		for (size_t i = 0; i < game->getNumAliens(); i++)
		{
			Alien* alien = game->getAlien(i);
			if (alien->getType() == ALIEN_DEAD)
				continue;

			SpriteAnimation animation = alienAnimation[alien->getType() - 1];
			size_t currentFrame = animation.getTime() / animation.getFrameDuration();
			Sprite* alienSprite = animation.getFrames()[currentFrame];

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
				}
				alien->setX(game->getWidth() - alienSprite->getWidth());
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
				alien->setX(0);
				alienMovementDirection *= -1;
				break;
			}
			else
			{
				alien->setX(alien->getX() + alienMovementDirection);
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
		buffer->drawSprite(*playerSprite, game->getPlayerX(), game->getPlayerY(), Buffer::rgb_to_uint(128, 0, 0));

		glTexSubImage2D(
			GL_TEXTURE_2D, 0, 0, 0,
			buffer->getWidth(), buffer->getHeight(),
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
			buffer->getData()
		);

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
		

		if (firePressed && game->getNumMissiles() < game->MAX_MISSILES)
		{
			game->fireMissile(playerSprite->getWidth(), playerSprite->getHeight(), 2);
			firePressed = false;
		}


		glfwSwapBuffers(window);

		glfwPollEvents();
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