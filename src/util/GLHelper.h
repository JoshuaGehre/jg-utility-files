#ifndef GLHELPER_H_DEFINED
#define GLHELPER_H_DEFINED

#include <GLInclude.h>

namespace GLHelper {
	// Set sampling filter and wrap for the selected texture
	void setTextureParameters(GLenum filter, GLenum wrap);
	// Flip an image vertically
	void flipImageY(unsigned char* pixels, unsigned int width, unsigned int height);
}; // namespace GLHelper

#endif
