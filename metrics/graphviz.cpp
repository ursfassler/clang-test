#include "graphviz.hpp"

std::string escape(const std::string& value)
{
	std::string result;

	result += "_";

	for (const auto& sym : value) {
			switch (sym) {
				case '_':
					result += "_5f";
					break;
				case ':':
					result += "_3a";
					break;
				case '=':
					result += "_3d";
					break;
				case '[':
					result += "_5b";
					break;
				case ']':
					result += "_5d";
					break;
				case '<':
					result += "_3c";
					break;
				case '>':
					result += "_3e";
					break;
				case '!':
					result += "_21";
					break;
				case '*':
					result += "_2a";
					break;
				case '+':
					result += "_2b";
					break;
				case '-':
					result += "_2d";
					break;
				case '.':
					result += "_2e";
					break;

				default:
					result += sym;
				}
		}

	return result;
}
