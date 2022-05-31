#include "BaseGlObject.h"

AttributeLocation::AttributeLocation() :
	size(0),
	offsetInStruct(0),
	offsetInGL(0),
	name(nullptr)
{}

AttributeLocation::AttributeLocation(const AttributeLocation& ALoc) :
	size(ALoc.size),
	offsetInStruct(ALoc.offsetInStruct),
	offsetInGL(ALoc.offsetInGL),
	name(ALoc.name)
{}

AttributeLocation::AttributeLocation(const char* name_, unsigned int size_, unsigned int offset_) :
	size(size_),
	offsetInStruct(offset_),
	offsetInGL(0),
	name(name_)
{}

AttributeLayout::AttributeLayout() :
	Attributes(0),
	totalSize(0)
{}

AttributeLayout::AttributeLayout(const AttributeLocation& ALoc) :
	AttributeLayout()
{
	append(ALoc);
}

AttributeLayout::AttributeLayout(const AttributeLayout& ALay) :
	Attributes(ALay.Attributes),
	totalSize(ALay.totalSize)
{}

AttributeLayout::~AttributeLayout()
{}

void AttributeLayout::append(const AttributeLocation& ALoc)
{
	unsigned int h = totalSize;
	totalSize += ALoc.size;
	Attributes.push_back(ALoc);
	Attributes[Attributes.size() - 1].offsetInGL = h;
}

IndexedTriangle::IndexedTriangle() :
	IndexedTriangle(0, 0, 0)
{}

IndexedTriangle::IndexedTriangle(unsigned int a_, unsigned int b_, unsigned int c_) :
	a(a_),
	b(b_),
	c(c_)
{}

IndexedTriangle IndexedTriangle::operator+(unsigned int off) const
{
	return IndexedTriangle(a + off, b + off, c + off);
}

bool BaseGlObject::adaptToShader()
{
	// Keep this
	bool wasPreviouslyCompatible = shaderCompatible;
	shaderCompatible = false;
	if(graphicsCardStatus == 0) return false;
	if((shaderInfo != nullptr) && (shaderInfo->useable)) {
		if(lastAdaptedShader == shaderInfo->id) {
			// Revert back to the compatibilty we had before
			shaderCompatible = wasPreviouslyCompatible;
			return true;
		}
		// Set the new shader
		lastAdaptedShader = shaderInfo->id;
		// Select vao vbo and eab
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eab);
		// Define how each attribute should be interpreted
		for(const AttributeLocation& A : Layout.getAttributes()) {
			unsigned int loc = glGetAttribLocation(lastAdaptedShader, A.name);
			if(loc == ((unsigned int) -1)) {
				// This attribute does not exist
				printf("%s does not exist as an attribute for the shader!\n", A.name);
				return false;
			}
			glEnableVertexAttribArray(loc);
			glVertexAttribPointer(
				loc,                                     // index
				A.size,                                  // size
				GL_FLOAT,                                // type
				GL_FALSE,                                // normalized
				sizeof(float) * Layout.size(),           // stride
				(void*) (sizeof(float) * A.offsetInGL)); // offset
		}
		shaderCompatible = true;
		return true;
	} else {
		lastAdaptedShader = (unsigned int) -1;
	}
	return false;
}

BaseGlObject::BaseGlObject(const AttributeLayout& L) :
	Layout(L),
	numberOfVertices(0),
	numberOfIndices(0),
	vertexData(0),
	indexData(0),
	graphicsCardStatus(0),
	lastAdaptedShader((unsigned int) -1),
	shaderCompatible(false),
	shaderInfo(nullptr),
	vao(0),
	vbo(0),
	trackVertices(true),
	vertexTracker([this](int64_t l, int64_t r) -> bool {
		return compareVertices(l, r);
	}),
	newVertexPointer(nullptr),
	vertexEpsilon(0.001f)
{}

BaseGlObject::~BaseGlObject()
{
	clearDataFromGraphicsCard();
	// Currently we aren't allocation
	// any other data explicitely, but
	// have the vectors deal with it
}

unsigned int BaseGlObject::addVertexF(const float* v)
{
	if(trackVertices) {
		newVertexPointer = v;
		std::map<int64_t, size_t>::iterator it = vertexTracker.find(-1);
		newVertexPointer = nullptr;
		if(it != vertexTracker.end()) {
			//printf("Found vertex at %d\n", (int) it->second);
			return it->second;
		}
	}
	unsigned int num = numberOfVertices;
	numberOfVertices++;
	for(const AttributeLocation& A : Layout.getAttributes()) {
		for(unsigned int i = 0; i < A.size; ++i) {
			vertexData.push_back(*(v + A.offsetInStruct + i));
		}
	}
	if(trackVertices) {
		vertexTracker.insert(std::pair<int64_t, size_t>(num, num));
	}
	return num;
}

void BaseGlObject::connectTriangle(
	unsigned int indexA,
	unsigned int indexB,
	unsigned int indexC)
{
	indexData.push_back(indexA);
	indexData.push_back(indexB);
	indexData.push_back(indexC);
	numberOfIndices += 3;
	if(graphicsCardStatus == 1) graphicsCardStatus = -1;
}

void BaseGlObject::addTriangleF(
	const float* a,
	const float* b,
	const float* c)
{
	unsigned int va = addVertexF(a);
	unsigned int vb = addVertexF(b);
	unsigned int vc = addVertexF(c);
	connectTriangle(va, vb, vc);
}

void BaseGlObject::connectQuadrangle(
	unsigned int indexA,
	unsigned int indexB,
	unsigned int indexC,
	unsigned int indexD)
{
	connectTriangle(indexA, indexB, indexC);
	connectTriangle(indexA, indexC, indexD);
}

void BaseGlObject::addQuadrangleF(
	const float* a,
	const float* b,
	const float* c,
	const float* d)
{
	unsigned int va = addVertexF(a);
	unsigned int vb = addVertexF(b);
	unsigned int vc = addVertexF(c);
	unsigned int vd = addVertexF(d);
	connectTriangle(va, vb, vc);
	connectTriangle(va, vc, vd);
}

bool BaseGlObject::copyDataToGraphicsCard()
{
	switch(graphicsCardStatus) {
		case 1:
			// Don't copy data again if it is allready
			// there and up to date
			return false;
		case -1:
			// Delete the outdated data first
			clearDataFromGraphicsCard();
		case 0:
			// Copy the data onto the graphics card
			// VAO
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
			// VBO
			unsigned int datasize = sizeof(float) * numberOfVertices * Layout.size();
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			if(vertexData.size()) {
				glBufferData(GL_ARRAY_BUFFER, datasize, &vertexData[0], GL_STATIC_DRAW);
			}
			// EAB
			datasize = sizeof(unsigned int) * numberOfIndices;
			glGenBuffers(1, &eab);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eab);
			if(indexData.size()) {
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, datasize, &indexData[0], GL_STATIC_DRAW);
			}
			// Mark the shader as unusable as vao vbo and eab have changed
			lastAdaptedShader = (unsigned int) -1;
			graphicsCardStatus = 1;
			return true;
	}
	// This should be unreachable
	return false;
}

bool BaseGlObject::clearDataFromGraphicsCard()
{
	if(graphicsCardStatus) {
		// I'm not sure if the order of these
		// matter but deleting them in reverse
		// order seems like the safest way
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &eab);
		glDeleteVertexArrays(1, &vao);
		graphicsCardStatus = 0;
		return true;
	}
	return false;
}

bool BaseGlObject::setShader(const SimpleShaderInfo* shader)
{
	shaderInfo = shader;
	return adaptToShader();
}

bool BaseGlObject::drawObject()
{
	// If the shader is usable and hasn't changed we can draw the
	// object, otherwise we first need to check if a shader is even
	// set, in order to not segfault. If it is set only try adapting
	// to it if it has changed so that we don't try adapting to an
	// unusable shader every time the object is drawn.
	// And if we now managed to adapt to it we should of course
	// also draw the object now.
	if((shaderCompatible && (lastAdaptedShader == shaderInfo->id)) || ((shaderInfo != nullptr) && (lastAdaptedShader != shaderInfo->id) && adaptToShader())) {
		// Select vao vbo and eab
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eab);
		// Select the shader
		glUseProgram(lastAdaptedShader);
		// Indexed drawing
		glDrawElements(GL_TRIANGLES, numberOfIndices, GL_UNSIGNED_INT, (void*) 0);
		return true;
	}
	return false;
}

bool BaseGlObject::compareVertices(int64_t l, int64_t r)
{
	for(unsigned int i = 0; i < Layout.size(); ++i) {
		float fl = getCompVertexElement(l, i);
		float fr = getCompVertexElement(r, i);
		if(fl + vertexEpsilon < fr) return true;
		if(fr + vertexEpsilon < fl) return false;
		// In case of equality the loop will
		// continue to the next element
	}
	return false;
}

float BaseGlObject::getCompVertexElement(int64_t i, unsigned int offset)
{
	return (i == -1) ? newVertexPointer[offset] : vertexData[i * Layout.size() + offset];
}

bool BaseGlObject::disableVertexTracking()
{
	if(!trackVertices) return false;
	trackVertices = false;
	vertexTracker.clear();
	return true;
}

bool BaseGlObject::enableVertexTracking(size_t start)
{
	if(trackVertices) return false;
	trackVertices = false;
	for(size_t i = start; i < numberOfVertices; i++) {
		vertexTracker.insert(std::pair<int64_t, size_t>((int64_t) i, i));
	}
	return true;
}

bool BaseGlObject::setEpsilon(float epsilon)
{
	if(epsilon < 0) return false;
	vertexEpsilon = epsilon;
	return true;
}
