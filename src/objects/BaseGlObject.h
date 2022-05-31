#ifndef BASEGLOBJECT_H_DEFINED
#define BASEGLOBJECT_H_DEFINED

#include <functional>
#include <map>
#include <vector>

#include "../shaders/Shaders.h"

// How to read an attribute from a struct
// And how to map it to the OpenGL arrays
// All sizes for floats
struct AttributeLocation {
	unsigned int size;
	unsigned int offsetInStruct;
	unsigned int offsetInGL;
	const char* name;
	AttributeLocation();
	AttributeLocation(const AttributeLocation& ALoc);
	AttributeLocation(const char* name_, unsigned int size_, unsigned int offset_);
};

// A shortcut for constructing an AttributeLocation
// This will only need the name of the struct and name of
// the struct memeber to get the size and offset of the
// member in the struct.
// The name of the attribute will be the same as the name
// of the struct member.
#define ATTRIB_LOC(STRUCT_NAME, MEMBER_NAME) AttributeLocation(#MEMBER_NAME,                                                  \
															   sizeof(((STRUCT_NAME*) nullptr)->MEMBER_NAME) / sizeof(float), \
															   offsetof(struct STRUCT_NAME, MEMBER_NAME) / sizeof(float))

// Full description of all attributes for an object
class AttributeLayout {
  private:
	std::vector<AttributeLocation> Attributes;
	unsigned int totalSize;

  public:
	AttributeLayout();
	AttributeLayout(const AttributeLocation& ALoc);
	AttributeLayout(const AttributeLayout& ALay);
	~AttributeLayout();
	// Add an Attribute Location and increase the size
	void append(const AttributeLocation& ALoc);
	// Access to the size
	inline unsigned int size() const { return totalSize; };
	// Access to attributes
	inline const std::vector<AttributeLocation> getAttributes() const { return Attributes; };
};

// Allow for an easy definition of an AttributeLayout by adding AttributeLocations
// Add two AttributeLocations resulting in an AttributeLayout
inline AttributeLayout operator+(const AttributeLocation& A, const AttributeLocation& B)
{
	AttributeLayout R(A);
	R.append(B);
	return R;
}

// Add an AtributeLocation to an AttributeLayout resulting in a AttributeLayout
// This is needed when we are adding more then two AttributeLocations
inline AttributeLayout operator+(const AttributeLayout& A, const AttributeLocation& B)
{
	AttributeLayout R(A);
	R.append(B);
	return R;
}

struct IndexedTriangle {
	unsigned int a;
	unsigned int b;
	unsigned int c;
	IndexedTriangle();
	IndexedTriangle(unsigned int a_, unsigned int b_, unsigned int c_);
	// Add an offset
	IndexedTriangle operator+(unsigned int off) const;
};

class BaseGlObject {
  private:
	const AttributeLayout Layout;
	// Could in theory be calculated from
	// the data vectors but whatever.
	// Might be useful if those are
	// rewritten at some point.
	unsigned int numberOfVertices;
	unsigned int numberOfIndices;
	// Add all data to vectors
	// There might be a better way to do this
	// but std::list allocates way more memory
	// per value for the linked list
	// Instead we should have large blocks and link them
	std::vector<float> vertexData;
	std::vector<unsigned int> indexData;
	// Is the data on the graphics card and up to date
	//  0 - No data on the graphics card
	//  1 - Data on the graphics card and up to date
	// -1 - Outdated data on the graphics card
	int graphicsCardStatus;
	// The last shader the object was adapted to
	// this alone is not allways safe to use in case
	// the shader has since then been changed
	unsigned int lastAdaptedShader;
	// A shader might be useable in general but
	// not compatible with the naming for the
	// attributes used by this object
	bool shaderCompatible;
	// Currently used shader info
	// If it changes this object should adept
	// If it is unusable we should not draw the object
	const SimpleShaderInfo* shaderInfo;
	// Adapt the object to a newly set shader
	// Or the current one if that one has changend
	bool adaptToShader();
	// Weird OpenGL Stuff
	// Vertex Array Object
	unsigned int vao;
	// Vertex Buffer Object
	unsigned int vbo;
	// Element Array Buffer
	unsigned int eab;
	// With this the object will check if a vertex allready exists
	bool trackVertices;
	std::map<int64_t, size_t, std::function<bool(int64_t, int64_t)>> vertexTracker;
	// Vertice comparison vertices at the indeced l and r
	// according to vertex(l) < vertex(r)
	// Where vertex(-1) refers to the buffer
	// for the new vertex
	bool compareVertices(int64_t l, int64_t r);
	float getCompVertexElement(int64_t i, unsigned int offset);
	const float* newVertexPointer;
	float vertexEpsilon;

  public:
	BaseGlObject(const AttributeLayout& L);
	~BaseGlObject();
	// Add a vertex to the object and return its index
	unsigned int addVertexF(const float* v);
	// Add a vertex but read it from a struct
	template <typename T>
	inline size_t addVertex(const T& v) { return addVertexF((float*) &v); };
	// Add a triangle
	// By the indeces
	void connectTriangle(unsigned int indexA, unsigned int indexB, unsigned int indexC);
	inline void connectTriangle(IndexedTriangle T) { connectTriangle(T.a, T.b, T.c); };
	// By the vertex data
	void addTriangleF(const float* a, const float* b, const float* c);
	template <typename T>
	inline void addTriangle(const T& a, const T& b, const T& c)
	{
		addTriangleF((float*) &a, (float*) &b, (float*) &c);
	};
	// Add a quadrangle by adding two triangles
	// both sharing the side BC
	// By the indeces
	void connectQuadrangle(unsigned int indexA, unsigned int indexB, unsigned int indexC, unsigned int indexD);
	// By the vertex data
	void addQuadrangleF(const float* a, const float* b, const float* c, const float* d);
	template <typename T>
	inline void addQuadrangle(const T& a, const T& b, const T& c, const T& d)
	{
		addQuadrangleF((float*) &a, (float*) &b, (float*) &c, (float*) &d);
	};
	// Access to the number of verticies and indices
	inline unsigned int sizeVertices() const { return numberOfVertices; };
	inline unsigned int sizeIndeces() const { return numberOfIndices; };
	// Copy data to the graphics card
	bool copyDataToGraphicsCard();
	// Clear data from the graphics card
	bool clearDataFromGraphicsCard();
	// Set the shader to be used by the object
	bool setShader(const SimpleShaderInfo* shader);
	// Draw the object
	bool drawObject();
	// Enable/disable vertex tracking
	// Disable and clear the map
	bool disableVertexTracking();
	// Enable from a certain starting point
	bool enableVertexTracking(size_t start);
	// Enable from now on
	inline bool enableVertexTrackingNow() { return enableVertexTracking(numberOfVertices); };
	// Enable vertex tracking retroactively
	inline bool enableVertexTrackingRetroactive() { return enableVertexTracking(0); };
	// Access to the epsilon value
	bool setEpsilon(float epsilon);
	inline float getEpsilon() const { return vertexEpsilon; };
};

#endif
