#include "GLHelper.h"

#include <algorithm>

void GLHelper::setTextureParameters(GLenum filter, GLenum wrap)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
}

void GLHelper::flipImageY(unsigned char* pixels, unsigned int width, unsigned int height)
{
	unsigned int h = height >> 1;
	unsigned int len = width << 2;
	for(unsigned int i = 0; i < h; i++) {
		std::swap_ranges(
			pixels + i * len,
			pixels + (i + 1) * len,
			pixels + (height - i - 1) * len);
	}
}
