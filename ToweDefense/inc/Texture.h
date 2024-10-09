#pragma once
#include <string>
#include "stb_image.h"
class Texture
{
private:
	unsigned id;

public:
	Texture();
	Texture(const std::string filePath);
	~Texture();
	void Bind();
	void Unbind();
	inline unsigned GetID() { return id; }
};

