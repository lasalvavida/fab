#pragma once

#include <string>
#include <fab/Configuration.h>
#include <fab/Object.h>

namespace fab {
	class Compiler {
	public:
		std::string runProcess(std::string command);
		std::string compile(fab::Configuration* config);
		virtual void buildObject(
			fab::Object* object,
			fab::Configuration* config) = 0;
		virtual std::string linkObjects(
			std::set<fab::Object*> objects,
			fab::Configuration* config) = 0;
	};
}