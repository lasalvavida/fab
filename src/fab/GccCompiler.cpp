#include <fab/GccCompiler.h>

#include <iostream>
#include <experimental/filesystem>

using namespace std;
using namespace std::experimental::filesystem;
using namespace fab;

string GccCompiler::baseCommand(Configuration* config) {
	string cmd = _gccPath + " -c";

	for (string includeDirectory : config->includeDirectories) {
		cmd += " -I" + includeDirectory;
	}

	if (config->cxxVersion == 11) {
		cmd += " -std=c++11";
	}

	return cmd;
}

void GccCompiler::buildObject(Object* object,
	                          Configuration* config) {
	string cmd = baseCommand(config);
	cmd += " " + object->sourceFile;

	path outputFilePath(object->objectFile);
	create_directories(outputFilePath.parent_path());

	cmd += " -o " + outputFilePath.string();

	cmd += "\n";
	cout << cmd;
	cout << runProcess(cmd);
}

set<string> getLinkLibraries(set<Object*> objects) {
	set<string> linkLibraries;
	for (Object* object : objects) {
		for (string include : object->includes) {
			if (include == "<experimental/filesystem>") {
				linkLibraries.insert("-lstdc++fs");
			}
		}
	}
	return linkLibraries;
}

string GccCompiler::linkObjects(set<Object*> objects,
								Configuration* config) {
	path outputFilePath = path(config->buildDirectory) /
		config->outputFile;

	string cmd = _gccPath + " -o " + outputFilePath.string();

	for (Object* object : objects) {
		cmd += " " + object->objectFile;
	}

	for (string linkLibrary : getLinkLibraries(objects)) {
		cmd += " " + linkLibrary;
	}

	create_directories(outputFilePath.parent_path());

	cmd += "\n";
	cout << cmd;
	cout << runProcess(cmd);

	return outputFilePath.string();
}