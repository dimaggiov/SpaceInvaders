#pragma once
#include "Sprite.h"
class SpriteAnimation
{
private:
	bool loop;
	size_t numFrames;
	size_t frameDuration;
	size_t time;
	Sprite** frames;

public:
	SpriteAnimation(size_t, size_t, size_t, Sprite**);

	bool isLooping();
	void toggleLoop();
	size_t getNumFrames();
	size_t getFrameDuration();
	size_t getTime();
	void addTime();
	void resetTime();
	Sprite** getFrames();
	
	
};

