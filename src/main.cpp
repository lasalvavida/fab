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
	map<string, Configuration*> libs;

	GccCompiler* compiler = new GccCompiler("g++");

	json::iterator findLib = configJson.find("lib");
	if (findLib != configJson.end()) {
		for (json lib : *findLib) {
			Configuration* config = new Configuration();
			string name = lib["name"];

			json::iterator findGit = lib.find("git");
			if (findGit != lib.end()) {
				string gitUrl = (*findGit).get<string>();

				path gitUrlPath = path(gitUrl);
				string repo = gitUrlPath.stem();
				string user = gitUrlPath.parent_path().filename();

				json::iterator findRef = lib.find("ref");
				string ref = "master";
				if (findRef != lib.end()) {
					ref = (*findRef).get<string>();
				}

				path outputDir = path(config->buildDirectory) /
					user / repo / ref;

				create_directories(outputDir);
				compiler->runProcess("git clone " + gitUrl + " " +
					outputDir.string() + "\n");

				config->sourceDirectory = outputDir.string();
			}

			json::iterator findExport = lib.find("export");
			if (findExport != lib.end()) {
				json exports = *findExport;
				json::iterator findInclude = exports.find("include");
				if (findInclude != exports.end()) {
					for (json include : *findInclude) {
						config->exportIncludeDirectories.insert(
							include.get<string>());
					}
				}
			}

			libs[name] = config;
		}
	}

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

			json::iterator findDepends = bin.find("depends");
			if (findDepends != bin.end()) {
				for (json depends : *findDepends) {
					string dependency = depends.get<string>();
					auto findLib = libs.find(dependency);
					if (findLib != libs.end()) {
						Configuration* dependencyConfig = findLib->second;
						for (string includeDirectory : dependencyConfig->exportIncludeDirectories) {
							path resolveInclude = path(dependencyConfig->sourceDirectory) /
								includeDirectory;
							config->includeDirectories.insert(resolveInclude.string());
						}
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

