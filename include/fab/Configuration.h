#pragma once

#include <set>
#include <string>

namespace fab {
	class Configuration {
	public:
		enum Type {
			EXECUTABLE,
			STATIC_LIBRARY
		};

		Type type;
		std::string buildDirectory = "build/";
		std::string sourceDirectory;
		std::set<std::string> sourceFiles;
		std::set<std::string> includeDirectories;
		unsigned int cxxVersion = 11;
		std::string outputFile;
		std::set<std::string> exportIncludeDirectories;
		std::set<Configuration*> dependencies;
	};
}