#pragma once
#include <filesystem>
#include <iosfwd>

namespace tanks::app {
struct Options {
	bool help = false;
	std::filesystem::path shaders;
};

Options parseOptions(int argc, char **argv);
void printControls(std::ostream &output);
void printUsage(std::ostream &output);
} // namespace tanks::app
