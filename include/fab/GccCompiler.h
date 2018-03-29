#pragma once

#include <fab/Compiler.h>

namespace fab {
	class GccCompiler : public Compiler {
	private:
		std::string _gccPath;
	protected:
		std::string baseCommand(fab::Configuration* config);
	public:
		GccCompiler(std::string gccPath) : _gccPath(gccPath) {};
		virtual void buildObject(
			fab::Object* object,
			fab::Configuration* config);
		virtual std::string linkObjects(
			std::set<fab::Object*> objects,
			fab::Configuration* config);
	};
}