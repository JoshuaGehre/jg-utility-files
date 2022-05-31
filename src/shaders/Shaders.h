#ifndef SHADERS_H_DEFINED
#define SHADERS_H_DEFINED

#include <string>
#include <vector>

#include <GLInclude.h>

struct SimpleShaderInfo;
class ShaderFile;
class ShaderProgram;

struct SimpleShaderInfo {
	unsigned int id = 0;
	bool useable = false;
	// Select this shader if it exists
	bool use() const;
	// Like use() but also draw a triangle
	// strip with four points, this serves
	// as a shortcut for applying
	// postProcessing shaders
	bool applyPostProcessing() const;
};

class ShaderFile {
  private:
	std::vector<ShaderProgram*> dependingPrograms;
	std::string fileName;
	bool isBuild;
	unsigned int id;
	GLenum shaderType;

  public:
	ShaderFile(std::string f);
	~ShaderFile();
	bool addDependingProgram(ShaderProgram* P);
	bool reload();
	void clean();
	inline bool isShaderBuild() { return isBuild; };
	inline unsigned int getId() { return id; };
};

class ShaderProgram {
  private:
	std::vector<ShaderFile*> dependencies;
	bool isBuild;
	SimpleShaderInfo& shaderInfo;
	unsigned int id;

  public:
	ShaderProgram(SimpleShaderInfo& info);
	~ShaderProgram();
	bool appendShader(ShaderFile* F);
	void clean();
	bool reload();
	inline bool isProgramBuild() { return isBuild; };
	inline unsigned int getId() { return id; };
};

#endif
