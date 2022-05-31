#include "UniformBufferObjects.h"

#include <algorithm>
#include <utility>

#include "../util/ExAlgorithm.h"
#include "../util/ExRandom.h"

GlobalUBOs::GlobalUBOs() :
	transforms_(),
	perlinNoise_()
{
	ExRandom Rand(1);
	// Initialize perlin noise
	UBOPerlinNoise& pn = perlinNoise_.get();
	// Create random unit vectors
	std::generate(std::begin(pn.vectors), std::end(pn.vectors),
				  [&]() { return glm::vec4(glm::vec3(Rand.getRandomUnitVector3D()), 0.0f); });
	ExAlg::indexedFor(pn.hash, [](int& val, size_t index) { val = index; });
	std::mt19937 mt(Rand.getUInt32());
	std::shuffle(std::begin(pn.hash), std::end(pn.hash), mt);
	// We aren't changing this during the game just set it once
	perlinNoise_.update();
}

void GlobalUBOs::update()
{
	transforms_.update();
	//perlinNoise_.update();
}
