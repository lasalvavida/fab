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

string gitClone(Compiler* compiler, string dir, string url, 
		string ref) {
	path urlPath = path(url);
	string repo = urlPath.stem();
	string user = urlPath.parent_path().filename();
	path outputDir = path(dir) / user / repo / ref;
	create_directories(outputDir);
	compiler->runProcess("git clone " + url + " " +
		outputDir.string() + "\n");
	return outputDir.string();
}

Configuration* createConfig(Compiler* compiler, json target,
		string sourceDirectory) {
	Configuration* config = new Configuration();
	config->sourceDirectory = sourceDirectory;

	json::iterator findGit = target.find("git");
	if (findGit != target.end()) {
		string url = (*findGit).get<string>();
		json::iterator findRef = target.find("ref");
		string ref = "master";
		if (findRef != target.end()) {
			ref = (*findRef).get<string>();
		}
		config->sourceDirectory = gitClone(compiler, 
			config->buildDirectory, url, ref);
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
					path dirPath = path(config->sourceDirectory) /
						dir.get<string>();
					for (const directory_entry entry :
							recursive_directory_iterator(
								dirPath)) {
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

json loadConfig(string configFile) {
	if (!exists(configFile)) {
		throw runtime_error(configFile + " does not exist.");
	}
	ifstream configFileStream(configFile);
	json configJson;
	configFileStream >> configJson;
	return configJson;
}

void createBuild(Compiler* compiler, json configJson, 
		vector<Configuration*>* builds,
		map<string, Configuration*>* libs,
		string sourceDirectory,
		bool root) {

	json::iterator findExternal = configJson.find("external");
	if (findExternal != configJson.end()) {
		for (json external : *findExternal) {
			json::iterator findGit = external.find("git");
			if (findGit != external.end()) {
				string url = (*findGit).get<string>();
				json::iterator findRef = external.find("ref");
				string ref = "master";
				if (findRef != external.end()) {
					ref = (*findRef).get<string>();
				}
				string dir = gitClone(compiler, "build", url, ref);
				json externalConfig = loadConfig(
					(path(dir) / "fab.json").string());
				createBuild(compiler, externalConfig,
					builds, libs, dir, false);
			}
		}
	}

	json::iterator findLib = configJson.find("lib");
	if (findLib != configJson.end()) {
		for (json lib : *findLib) {
			Configuration* config = createConfig(compiler, lib,
				sourceDirectory);
			config->type = Configuration::Type::STATIC_LIBRARY;
			string name = lib["name"];
			if (config->sourceFiles.size() > 0) {
				config->outputFile = name;
				builds->push_back(config);
			}
			libs->emplace(name, config);
		}
	}

	if (root) {
		json::iterator findBin = configJson.find("bin");
		if (findBin != configJson.end()) {
			for (json bin : *findBin) {
				Configuration* config = createConfig(compiler, bin,
					sourceDirectory);
				config->type = Configuration::Type::EXECUTABLE;
				config->outputFile = bin["name"];

				json::iterator findDepends = bin.find("depends");
				if (findDepends != bin.end()) {
					for (json depends : *findDepends) {
						string dependency = depends.get<string>();
						auto findLib = libs->find(dependency);
						if (findLib != libs->end()) {
							config->dependencies.insert(findLib->second);
						} else {
							cout << "Dependency " << dependency << " not found." << endl;
						}
					}
				}
				builds->push_back(config);
			}
		}
	}
}

int main() {
	json configJson = loadConfig("fab.json");

	vector<Configuration*> builds;
	map<string, Configuration*> libs;

	GccCompiler* compiler = new GccCompiler("g++", "ar");
	createBuild(compiler, configJson, &builds, &libs, "", true);
	
	for (Configuration* build : builds) {
		compiler->compile(build);
	}
}

