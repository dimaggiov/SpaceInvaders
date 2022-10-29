#pragma once
#include "Sprite.h"
#include "SpriteAnimation.h"

#ifndef SPACE_INVADERS_SPRITES
#define SPACE_INVADERS_SPRITES

namespace sprites {
	extern Sprite* alienSprites[6];
	extern Sprite* deadAlienSprite;
	extern SpriteAnimation* alienAnimation[3];
	extern Sprite* playerSprite;
	extern Sprite* missileSprite;
	extern Sprite* bunkerSprites[3];
	extern Sprite* textSpriteSheet;
	extern Sprite* numberSpriteSheet;
}
#endif
