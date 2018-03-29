#include <fab/Compiler.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <memory>
#include <experimental/filesystem>

using namespace std;
using namespace std::experimental::filesystem;
using namespace fab;

const string includeMacro = "#include";

inline string trim(string& str) {
	size_t lastLength = 0;
	while (str.length() != lastLength) {
		lastLength = str.size();
		str.erase(0, str.find_first_not_of(' '));
		str.erase(0, str.find_first_not_of('\t'));
		str.erase(str.find_last_not_of(' ') + 1);
		str.erase(str.find_last_not_of('\t') + 1);
	}
	return str;
}

string Compiler::runProcess(string command) {
	string output;
	command += "2>&1";
	const int bufferSize = 128;
	array<char, bufferSize> buffer;
	shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
	if (pipe == NULL) {
		throw runtime_error("Failed to execute: " + command);
	}
	while (!feof(pipe.get())) {
		if (fgets(buffer.data(), bufferSize, pipe.get()) != NULL) {
			output += buffer.data();
		}
	}
	return output;
}

Object* createObject(string sourceFile,
	                 Configuration* config) {
	Object* object = new Object();
	object->sourceFile = sourceFile;

	path outputFilePath(sourceFile);
	outputFilePath.replace_extension(".o");
	outputFilePath = path(config->buildDirectory) /
		outputFilePath;
	object->objectFile = outputFilePath.string();

	ifstream sourceStream(sourceFile);
	for (string line; getline(sourceStream, line); ) {
		line = trim(line);
		// This isn't very smart, but good enough for now
		if (line.substr(0, includeMacro.size()) == includeMacro) {
			size_t openCarat = line.find("<");
			size_t closeCarat = line.find(">");
			string include = line.substr(openCarat, closeCarat);
			object->includes.push_back(include);
		}
	}

	return object;
}

string Compiler::compile(Configuration* config) {
	set<Object*> objects;
	for (string sourceFile : config->sourceFiles) {
		Object* object = createObject(sourceFile, config);
		buildObject(object, config);
		objects.insert(object);
	}
	return linkObjects(objects, config);
}
