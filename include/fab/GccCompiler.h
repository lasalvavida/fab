#pragma once

#include <fab/Compiler.h>

namespace fab {
	class GccCompiler : public Compiler {
	private:
		std::string _gccPath;
		std::string _arPath;
	protected:
		std::string baseCommand(fab::Configuration* config);
	public:
		GccCompiler(std::string gccPath, std::string arPath) : 
			_gccPath(gccPath), _arPath(arPath) {};
		virtual void buildObject(
			fab::Object* object,
			fab::Configuration* config);
		virtual std::string linkObjects(
			std::set<fab::Object*> objects,
			fab::Configuration* config);
	};
}