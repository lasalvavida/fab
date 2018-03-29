#pragma once

#include <string>
#include <vector>

namespace fab {
	class Object {
	public:
		std::string sourceFile;
		std::string objectFile;
		std::vector<std::string> includes;
	};
}