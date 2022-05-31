#include "ExRandom.h"

#define _USE_MATH_DEFINES
#include <cmath>

unsigned int generateRandomSeed()
{
	std::random_device genSeed;
	std::uniform_int_distribution<unsigned int> distSeed(0, 0xffffffff);
	return distSeed(genSeed);
}

ExRandom::ExRandom() :
	ExRandom(generateRandomSeed())
{}

ExRandom::ExRandom(unsigned int seed) :
	gen_Rnd(seed),
	dist_Double_0_1(0, 1),
	dist_UInt_32Bit(0, 0xffffffff),
	hasRandNormalBuffered(false),
	randNormalBuffer(0)
{}

double ExRandom::getDoubleNormal()
{
	if(hasRandNormalBuffered) {
		hasRandNormalBuffered = false;
		return randNormalBuffer;
	} else {
		// Generate two standart normally distribued
		// numbers using a box muller transform
		double u = 1 - getDouble01(); // (0, 1]
		double phi = 2 * M_PI * getDouble01();
		double r = sqrt(-2 * log(u));
		// Keep one number for the next call and return the other one
		hasRandNormalBuffered = true;
		randNormalBuffer = r * sin(phi);
		return r * cos(phi);
	}
}

glm::dvec3 ExRandom::getRandomUnitVector3D()
{
	double z = 2 * getDouble01() - 1;
	double phi = 2 * M_PI * getDouble01();
	double xy = sqrt(1 - z * z);
	return glm::dvec3(cos(phi) * xy, sin(phi) * xy, z);
}

void ExRandom::genRandUnitVectorND(double* vec, unsigned int dim)
{
	double s = 0;
	for(unsigned int i = 0; i < dim; i++) {
		double x = getDoubleNormal();
		vec[i] = x;
		s += x * x;
	}
	s = 1.0 / sqrt(s);
	for(unsigned int i = 0; i < dim; i++) {
		vec[i] *= s;
	}
}

ExRandom* ExRandom::newExRandom()
{
	return new ExRandom(getUInt32());
}
