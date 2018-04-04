#include <fab/Compiler.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <memory>
#include <experimental/filesystem>

#include <sugar/Parser.h>

using namespace std;
using namespace std::experimental::filesystem;
using namespace fab;
using namespace sugar;

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
	string sourceContent((istreambuf_iterator<char>(sourceStream)),
		(istreambuf_iterator<char>()));

	Parser* parser = new Parser();
	Block* block = parser->parse(sourceContent);
	object->includes = block->getIncludes();

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
