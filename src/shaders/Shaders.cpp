#include "Shaders.h"

#include <fstream>
#include <iostream>

bool SimpleShaderInfo::use() const
{
	if(useable) glUseProgram(id);
	return useable;
}

bool SimpleShaderInfo::applyPostProcessing() const
{
	if(useable) {
		glUseProgram(id);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	return useable;
}

GLenum determineShaderType(const std::string& f)
{
	size_t endingStart = f.rfind('.');
	if((endingStart == std::string::npos) || (endingStart == (f.size() - 1))) {
		throw std::invalid_argument(("Could not determine ending of: " + f + "\n").c_str());
	}
	std::string ending = f.substr(endingStart + 1);
	if(ending == "frag") return GL_FRAGMENT_SHADER;
	if(ending == "vert") return GL_VERTEX_SHADER;
	throw std::invalid_argument(("Unknown ending \"" + ending + "\" for: " + f + "\n").c_str());
}

ShaderFile::ShaderFile(std::string f) :
	dependingPrograms(0),
	fileName(f),
	isBuild(false),
	id(0),
	shaderType(determineShaderType(f))
{
	reload();
}

ShaderFile::~ShaderFile()
{
	clean();
}

bool ShaderFile::addDependingProgram(ShaderProgram* P)
{
	for(ShaderProgram* S : dependingPrograms) {
		if(S == P) return false;
	}
	dependingPrograms.push_back(P);
	return true;
}

bool ShaderFile::reload()
{
	clean();
	// Open the file
	std::ifstream In;
	In.open(fileName);
	if(!In) {
		printf("Error: %s does not exist!\n", fileName.c_str());
	} else {
		// Read all content from the file
		std::string fileContent = "";
		std::string line;
		while(!In.eof()) {
			getline(In, line);
			fileContent.append(line).append("\n");
		}
		In.close();
		// Create a shader
		id = glCreateShader(shaderType);
		// Compile
		const char* fileContentChar = fileContent.c_str();
		glShaderSource(id, 1, &fileContentChar, NULL);
		glCompileShader(id);
		// Check if compiling was successful
		GLint isCompiled = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &isCompiled);
		if(isCompiled == GL_FALSE) {
			printf("Error: Can't compile %s\n", fileName.c_str());
			printf("Code:\n%s\n", fileContentChar);
			// Print the info log for the sahder
			GLint maxLength = 0;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> errorLog(maxLength);
			glGetShaderInfoLog(id, maxLength, &maxLength, &errorLog[0]);
			for(GLchar c : errorLog)
				printf("%c", (char) c);
			printf("\n");
			// Delete the shader
			glDeleteShader(id);
		} else {
			// No issues, this is now safe to use
			isBuild = true;
			return true;
		}
	}
	// Load the shader
	return false;
}

void ShaderFile::clean()
{
	if(!isBuild) return;
	// Clean all programs that use this shader
	for(ShaderProgram* p : dependingPrograms) {
		p->clean();
	}
	// Delete this shader
	glDeleteShader(id);
	// Mark as not build
	isBuild = false;
}

ShaderProgram::ShaderProgram(SimpleShaderInfo& info) :
	dependencies(0),
	isBuild(false),
	shaderInfo(info),
	id(0)
{}

ShaderProgram::~ShaderProgram()
{
	clean();
}

bool ShaderProgram::appendShader(ShaderFile* F)
{
	for(ShaderFile* S : dependencies) {
		if(S == F) return false;
	}
	dependencies.push_back(F);
	F->addDependingProgram(this);
	return true;
}

void ShaderProgram::clean()
{
	if(!isBuild) return;
	// Mark as unusable
	shaderInfo.useable = false;
	// Delete from graphics card
	// Detaching ?
	//...
	glDeleteProgram(id);
	isBuild = false;
}

bool ShaderProgram::reload()
{
	clean();
	// Check if we can build this
	for(ShaderFile* S : dependencies) {
		if(!S->isShaderBuild()) return false;
	}
	// Create the program
	id = glCreateProgram();
	for(ShaderFile* f : dependencies) {
		glAttachShader(id, f->getId());
	}
	glLinkProgram(id);
	shaderInfo.id = id;
	shaderInfo.useable = true;
	return true;
}
