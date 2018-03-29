#pragma once

#include <set>
#include <string>

namespace fab {
	class Configuration {
	public:
		std::string buildDirectory = "build/";
		std::set<std::string> sourceFiles;
		std::set<std::string> includeDirectories;
		unsigned int cxxVersion = 11;
		std::string outputFile;
	};
}