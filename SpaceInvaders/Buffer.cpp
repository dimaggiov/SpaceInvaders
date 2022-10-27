#include "Buffer.h"

Buffer::Buffer(size_t width, size_t height)
{
	this->width = width;
	this->height = height;
	this->data = nullptr;
}

Buffer::~Buffer()
{
	delete[] data;
}

uint32_t * Buffer::getData() const
{
	return data;
}

void Buffer::setData(uint32_t * data)
{
	this->data = data;
}

const uint32_t Buffer::rgb_to_uint(uint8_t r, uint8_t g, uint8_t b)
{
	return (r << 24) | (g << 16) | (b << 8) | 255;
}

void Buffer::clearBuffer(uint32_t color)
{
	for (size_t i = 0; i < width * height; ++i)
	{
		data[i] = color;
	}
}

void Buffer::drawSprite(const Sprite& sprite, size_t x, size_t y, uint32_t color)
{
	uint8_t *spriteData = sprite.getData();
	for (size_t xi = 0; xi < sprite.getWidth(); ++xi)
	{
		for (size_t yi = 0; yi < sprite.getHeight(); ++yi)
		{
			size_t sy = sprite.getHeight() - 1 + y - yi;
			size_t sx = x + xi;

			//if the pixel is turned on and it is on screen set it to the color
			if (spriteData[(yi * sprite.getWidth()) + xi] && sy < this->height && sx < this->width)
			{
				this->data[sy * this->width + sx] = color;
			}
		}
	}
}

void Buffer::drawText(const Sprite& spriteSheet, const char* text, size_t x, size_t y, uint32_t color)
{
	size_t xp = x;
	size_t stride = spriteSheet.getWidth() * spriteSheet.getHeight();
	Sprite sprite = spriteSheet;
	for (const char* charp = text; *charp != '\0'; charp++)
	{
		char character = *charp - 32;
		if (character < 0 || character >= 65)
			continue;
		
		sprite.setData(spriteSheet.getData() + character * stride);
		drawSprite(sprite, xp, y, color);
		xp += sprite.getWidth() + 1;
	}
}

void Buffer::drawNumber(const Sprite& spriteSheet, size_t number, size_t x, size_t y, uint32_t color)
{
	uint8_t digits[64];
	size_t numDigits = 0;

	size_t currentNumber = number;
	do
	{
		digits[numDigits++] = currentNumber % 10;
		currentNumber = currentNumber / 10;
	} while (currentNumber > 0);

	size_t xp = x;
	size_t stride = spriteSheet.getWidth() * spriteSheet.getHeight();
	Sprite sprite = spriteSheet;
	for (size_t i = 0; i < numDigits; i++)
	{
		uint8_t digit = digits[numDigits - i - 1];
		sprite.setData(spriteSheet.getData() + digit * stride);
		drawSprite(sprite, xp, y, color);
		xp += sprite.getWidth() + 1;
	}

}

size_t Buffer::getWidth() const
{
	return width;
}

void Buffer::setWidth(size_t width)
{
	this->width = width;
}

size_t Buffer::getHeight() const
{
	return height;
}

void Buffer::setHeight(size_t)
{
	this->height = height;
}
