#include "SpriteAnimation.h"

SpriteAnimation::SpriteAnimation(size_t numFrames, size_t frameDuration, size_t time, Sprite** frames)
{
	this->loop = true;
	this->numFrames = numFrames;
	this->frameDuration = frameDuration;
	this->time = time;
	this->frames = frames;
}

bool SpriteAnimation::isLooping()
{
	return loop;
}

void SpriteAnimation::toggleLoop()
{
	loop = !loop;
}

size_t SpriteAnimation::getNumFrames()
{
	return numFrames;
}

size_t SpriteAnimation::getFrameDuration()
{
	return frameDuration;
}

size_t SpriteAnimation::getTime()
{
	return time;
}

void SpriteAnimation::addTime()
{
	this->time++;
}

void SpriteAnimation::resetTime()
{
	time = 0;
}

Sprite** SpriteAnimation::getFrames()
{
	return frames;
}
