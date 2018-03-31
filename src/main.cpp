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

Configuration* createConfig(Compiler* compiler, json target) {
	Configuration* config = new Configuration();

	json::iterator findGit = target.find("git");
	if (findGit != target.end()) {
		string gitUrl = (*findGit).get<string>();

		path gitUrlPath = path(gitUrl);
		string repo = gitUrlPath.stem();
		string user = gitUrlPath.parent_path().filename();

		json::iterator findRef = target.find("ref");
		string ref = "master";
		if (findRef != target.end()) {
			ref = (*findRef).get<string>();
		}

		path outputDir = path(config->buildDirectory) /
			user / repo / ref;

		create_directories(outputDir);
		compiler->runProcess("git clone " + gitUrl + " " +
			outputDir.string() + "\n");

		config->sourceDirectory = outputDir.string();
	}

	json::iterator findExport = target.find("export");
	if (findExport != target.end()) {
		json exports = *findExport;
		json::iterator findInclude = exports.find("include");
		if (findInclude != exports.end()) {
			for (json include : *findInclude) {
				config->exportIncludeDirectories.insert(
					include.get<string>());
			}
		}
	}

	json::iterator findInclude = target.find("include");
	if (findInclude != target.end()) {
		for (json include : *findInclude) {
			config->includeDirectories.insert(include.get<string>());
		}
	}

	json::iterator findSources = target.find("sources");
	if (findSources != target.end()) {
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

	return config;
}

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
	map<string, Configuration*> libs;

	GccCompiler* compiler = new GccCompiler("g++", "ar");

	json::iterator findLib = configJson.find("lib");
	if (findLib != configJson.end()) {
		for (json lib : *findLib) {
			Configuration* config = createConfig(compiler, lib);
			config->type = Configuration::Type::STATIC_LIBRARY;
			string name = lib["name"];

			if (config->sourceFiles.size() > 0) {
				config->outputFile = name;
				builds.push_back(config);
			}

			libs[name] = config;
		}
	}

	json::iterator findBin = configJson.find("bin");
	if (findBin != configJson.end()) {
		for (json bin : *findBin) {
			Configuration* config = createConfig(compiler, bin);
			config->type = Configuration::Type::EXECUTABLE;
			config->outputFile = bin["name"];

			json::iterator findDepends = bin.find("depends");
			if (findDepends != bin.end()) {
				for (json depends : *findDepends) {
					string dependency = depends.get<string>();
					auto findLib = libs.find(dependency);
					if (findLib != libs.end()) {
						config->dependencies.insert(findLib->second);
					} else {
						cout << "Dependency " << dependency << " not found." << endl;
					}
				}
			}

			builds.push_back(config);
		}
	}

	for (Configuration* build : builds) {
		compiler->compile(build);
	}
}

