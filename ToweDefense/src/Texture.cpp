#include "Texture.h"
#include "glew.h"
#include <iostream>

Texture::Texture()
	: id(0) 
{
}

Texture::Texture(const std::string filePath)
	: id(0)
{
	
	glActiveTexture(GL_TEXTURE0);
	int width, height, bpp;

	stbi_set_flip_vertically_on_load(1);
	unsigned char* texBuffer = stbi_load(filePath.c_str(), &width, &height, &bpp, 4);
	if (!texBuffer) std::cout << "Texture at path: " << filePath << " failed to load!" << std::endl;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texBuffer);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (texBuffer)
		stbi_image_free(texBuffer);

}
Texture::~Texture()
{
	glDeleteTextures(1, &id);
	std::cout << "Destructing Texture with ID: " << id << std::endl;
}
void Texture::Bind() {

	glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::Unbind() {

	glBindTexture(GL_TEXTURE_2D, 0);
}