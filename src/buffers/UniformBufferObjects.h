#ifndef UNIFORM_BUFFER_OBJECTS_H_DEFINED
#define UNIFORM_BUFFER_OBJECTS_H_DEFINED

#include <GLInclude.h>

#include <glm/gtc/type_ptr.hpp>

#define UBO_TRANSFORM_BINDING       0
#define UBO_PERLIN_NOISE_BINDING    1

struct UBOTransforms {
	glm::mat4x4 toWorldSpace;
	glm::mat4x4 perspective;
};

struct UBOPerlinNoise {
	int hash[256];
	glm::vec4 vectors[256];
};

template <typename T>
class UniformBufferObject {
  private:
	T ubo;
	unsigned int id;
	bool dirty;

  public:
	UniformBufferObject();
	~UniformBufferObject();
	inline const T& read() const { return ubo; };
	inline T& get()
	{
		dirty = true;
		return ubo;
	};
	void update();
	inline void bind();
	// Find the binding for this type
	// Only for the specific types above
	inline unsigned int getBinding();
};

class GlobalUBOs {
  private:
	// UBO's
	UniformBufferObject<UBOTransforms> transforms_;
	UniformBufferObject<UBOPerlinNoise> perlinNoise_;

  public:
	GlobalUBOs();
	~GlobalUBOs() = default;
	// Access to the members
	UniformBufferObject<UBOTransforms>& transforms() { return transforms_; };
	const UniformBufferObject<UBOPerlinNoise>& perlinNoise() { return perlinNoise_; };

	// Update all UBOs
	void update();
};

template <typename T>
UniformBufferObject<T>::UniformBufferObject() :
	dirty(true)
{
	// Create the ubo
	glGenBuffers(1, &id);
	bind();
	glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, GL_STATIC_DRAW);
}

template <typename T>
UniformBufferObject<T>::~UniformBufferObject()
{
	// Delete the ubo
	glDeleteBuffers(1, &id);
}

template <typename T>
inline void UniformBufferObject<T>::bind()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, getBinding(), id);
	glBindBuffer(GL_UNIFORM, id);
}

template <typename T>
void UniformBufferObject<T>::update()
{
	bind();
	if(dirty) {
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), &ubo);
		dirty = false;
	}
}

#define SET_TYPE_BINDING(T, B) \
	template <>                \
	inline unsigned int UniformBufferObject<T>::getBinding() { return B; };

SET_TYPE_BINDING(UBOTransforms, UBO_TRANSFORM_BINDING)
SET_TYPE_BINDING(UBOPerlinNoise, UBO_PERLIN_NOISE_BINDING)

#undef SET_TYPE_BINDING

#endif
