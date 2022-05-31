#ifndef EXRANDOM_H_DEFINED
#define EXRANDOM_H_DEFINED

#include <random>

#include <glm/gtc/type_ptr.hpp>

class ExRandom {
  private:
	std::default_random_engine gen_Rnd;
	// Random Double 0 - 1
	std::uniform_real_distribution<double> dist_Double_0_1;
	// Random UInt 32 bit
	std::uniform_int_distribution<unsigned int> dist_UInt_32Bit;
	// The box muller transform will create 2
	// normal distributed numbers at a time so
	// we can keep one buffered and return it
	// the next time
	bool hasRandNormalBuffered;
	double randNormalBuffer;

  public:
	ExRandom();
	ExRandom(unsigned int seed);
	// Get Random Numbers
	// Uniformly distributed random number in [0, 1)
	inline double getDouble01() { return dist_Double_0_1(gen_Rnd); };
	// Uniformly distributed unsigned int in [0, 2^32-1]
	inline unsigned int getUInt32() { return dist_UInt_32Bit(gen_Rnd); };
	// Standard normal distribution
	double getDoubleNormal();
	// Random 3D unit vector
	glm::dvec3 getRandomUnitVector3D();
	// Random N dimensional unit vector
	// Pass the array by reference
	void genRandUnitVectorND(double* vec, unsigned int dim);
	// Create a new random number generator with a "random" seed
	ExRandom* newExRandom();
};

#endif
