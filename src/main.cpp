#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <fab/Configuration.h>
#include <fab/GccCompiler.h>
#include <nlohmann/json.hpp>

using namespace std;
using namespace std::experimental::filesystem;
using namespace fab;
using namespace nlohmann;

int main() {
	string configFile = "fab.json";
	if (!exists(configFile)) {
		cout << configFile << " does not exist." << endl;
		return 1;
	}

	ifstream configFileStream(configFile);
	json configJson;
	configFileStream >> configJson;

	vector<Configuration*> builds;

	json::iterator findBin = configJson.find("bin");
	if (findBin != configJson.end()) {
		for (json bin : *findBin) {
			Configuration* config = new Configuration();
			string name = bin["name"];
			config->outputFile = name;

			json::iterator findInclude = bin.find("include");
			if (findInclude != bin.end()) {
				for (json include : *findInclude) {
					config->includeDirectories.insert(include.get<string>());
				}
			}

			json::iterator findSources = bin.find("sources");
			if (findSources != bin.end()) {
				for (json source : *findSources) {
					json::iterator findDir = source.find("dir");
					if (findDir != source.end()) {
						json dir = *findDir;
						bool recursive = false;
						json::iterator findRecursive = source.find("recursive");
						if (findRecursive != source.end()) {
							recursive = (*findRecursive).get<bool>();
						}

						if (recursive) {
							for (const directory_entry entry :
									recursive_directory_iterator(
										dir.get<string>())) {
								path p = entry.path();
								if (!is_directory(p) &&
										p.extension() == ".cpp") {
									config->sourceFiles.insert(p.string());
								}
							}
						}
					}
				}
			}
			builds.push_back(config);
		}
	}

	GccCompiler* compiler = new GccCompiler("g++");
	for (Configuration* build : builds) {
		compiler->compile(build);
	}
}

