#define GLEW_STATIC
#include "texture.h"
#include "../gl.h"
#include "stb_image.h"
#include <iostream>

void RectCopy
(
	const unsigned char nChannels,
	const unsigned char *const src, const size_t srcWidth,
	unsigned char * dest, const size_t destWidth, const size_t dsetHeight,
	const size_t xStartSrc, const size_t yStartSrc
)
{
	size_t xLimit = xStartSrc + destWidth;
	size_t yLimit = yStartSrc + dsetHeight;
	for (size_t y = yStartSrc; y < yLimit; ++y)
	{
		for (size_t x = xStartSrc; x < xLimit; ++x)
		{
			for (unsigned char c = 0; c < nChannels; ++c)
			{
				*dest++ = src[(y * srcWidth + x) * 4 + c];
			}
		}
	}
}

Texture::Texture(const std::string& fileName, const int dim)
{
	int width, height, numComponents;
	unsigned char* data;
	
      
    glGenTextures(1, &m_texture);
	texDimention = dim;
	Bind(m_texture);
	switch (dim)
	{
	case 1:
		data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4);
		if (data == NULL)
			std::cerr << "Unable to load texture: " << fileName << std::endl;

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		break;
	case 2:
	default:
		data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4);
		if (data == NULL)
			std::cerr << "Unable to load texture: " << fileName << std::endl;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.4f);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		break;
	case 3://cube map
		
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.4f);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		constexpr unsigned char nChannels = 4;
		data = stbi_load(fileName.c_str(), &width, &height, &numComponents, nChannels);
		size_t faceWidth = width / 4, faceHeight = height / 3;

		auto buffer = std::make_unique<unsigned char[]>(faceWidth * faceHeight * nChannels);
		auto SetupCubeFace =
			[&nChannels , &data, &width, &buffer, &faceWidth, &faceHeight]
			(unsigned char faceIndex, unsigned char xOffsetMultiplier, unsigned char yOffsetMultiplier)
		{
			RectCopy
			(
				nChannels,
				data, width,
				buffer.get(), faceWidth, faceHeight,
				xOffsetMultiplier * faceWidth , yOffsetMultiplier * faceHeight
			);
			glTexImage2D
			(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex, 0,
				GL_RGBA, faceWidth, faceHeight, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, buffer.get()
			);
		};

		SetupCubeFace(0, 2, 1); // Right
		SetupCubeFace(1, 0, 1); // Left
		SetupCubeFace(2, 1, 0); // Top
		SetupCubeFace(3, 1, 2); // Bottom
		SetupCubeFace(4, 1, 1); // Front
		SetupCubeFace(5, 3, 1); // Back

		// glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, 0, GL_RGBA, faceWidth, faceHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		// glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1, 0, GL_RGBA, faceWidth, faceHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		// glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, 0, GL_RGBA, faceWidth, faceHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		// glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3, 0, GL_RGBA, faceWidth, faceHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		// glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4, 0, GL_RGBA, faceWidth, faceHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		// glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5, 0, GL_RGBA, faceWidth, faceHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		// 
		// glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, 0, faceWidth * 2, faceHeight, faceWidth, faceHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
		// glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1, 0, 0, faceHeight, faceWidth, faceHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
		// glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, 0, faceWidth, 0, faceWidth, faceHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
		// glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3, 0, faceWidth, faceHeight * 2, faceWidth, faceHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
		// glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4, 0, faceWidth, faceHeight, faceWidth, faceHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
		// glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5, 0, faceWidth * 3, faceHeight, faceWidth, faceHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);

		// std::string directions[] = { "Right","Left","Top","Bottom","Front","Back" };
		// for (int y = 0; y < 6; y++)
		// {
		// 	data = stbi_load((fileName + directions[y] + ".bmp").c_str(), &width, &height, &numComponents, 4);
		// 	if (data == NULL)
		// 		std::cerr << "Unable to load texture: " << fileName << std::endl;
		// 
		// 	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		// }
	}
	stbi_image_free(data);
}

Texture::Texture(int width,int height,unsigned char *data)
{
	glGenTextures(1, &m_texture);
	if (height > 0)
	{
		texDimention = 2;
		Bind(m_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else
	{ 
		texDimention = 1;
		Bind(m_texture);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}	
}

Texture::Texture(int width, int height)
{
	glGenTextures(1, &m_texture);
	if (height > 0)
	{
		texDimention = 2;
		Bind(m_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		texDimention = 1;
		Bind(m_texture);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	}
}


Texture::~Texture()
{
	glDeleteTextures(1, &m_texture);
}

void Texture::Bind(int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	switch (texDimention)
	{
	case 1:
		glBindTexture(GL_TEXTURE_1D, m_texture);
		break;
	case 2:
	default:
		//int tex = 1;
		glBindTexture(GL_TEXTURE_2D, m_texture);
		break;
	case 3:
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);
	}
}


