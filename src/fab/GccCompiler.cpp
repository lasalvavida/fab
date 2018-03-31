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

	for (Configuration* dependency : config->dependencies) {
		for (string includeDirectory : dependency->exportIncludeDirectories) {
			path resolveInclude = path(dependency->sourceDirectory) /
				includeDirectory;
			cmd += " -I" + resolveInclude.string();
		}
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
	if (config->type == Configuration::Type::STATIC_LIBRARY) {
		config->outputFile = path(config->outputFile)
			.replace_extension(".a").string();
	}

	path outputFilePath = path(config->buildDirectory) /
		config->outputFile;

	string cmd;
	if (config->type == Configuration::Type::STATIC_LIBRARY) {
		cmd = _arPath + " rcs " + outputFilePath.string();

		for (Object* object : objects) {
			cmd += " " + object->objectFile;
		}
	} else {
		cmd = _gccPath + " -o " + outputFilePath.string();

		for (Object* object : objects) {
			cmd += " " + object->objectFile;
		}

		for (Configuration* dependency : config->dependencies) {
			if (dependency->outputFile.size() > 0) {
				cmd += " " + (path(config->buildDirectory) / 
					dependency->outputFile).string();
			}
		}

		for (string linkLibrary : getLinkLibraries(objects)) {
			cmd += " " + linkLibrary;
		}
	}

	create_directories(outputFilePath.parent_path());

	cmd += "\n";
	cout << cmd;
	cout << runProcess(cmd);

	return outputFilePath.string();
}