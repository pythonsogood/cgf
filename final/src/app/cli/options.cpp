#include "app/cli/options.hpp"
#include <array>
#include <ostream>
#include <stdexcept>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif

namespace tanks::app {
namespace {
std::filesystem::path executableDirectory([[maybe_unused]] const char *argv0) {
#ifdef _WIN32
	std::array<wchar_t, 32768> path{};
	const DWORD size =
	    GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
	if (size == 0 || size == path.size())
		throw std::runtime_error("Cannot locate executable directory");
	return std::filesystem::path(std::wstring(path.data(), size)).parent_path();
#else
	std::error_code error;
	const auto path = std::filesystem::read_symlink("/proc/self/exe", error);
	if (!error)
		return path.parent_path();
	return std::filesystem::absolute(argv0).parent_path();
#endif
}
} // namespace
Options parseOptions(int argc, char **argv) {
	Options result;
	result.shaders = executableDirectory(argv[0]) / "shaders";

	for (int i = 1; i < argc; ++i) {
		const std::string option = argv[i];
		if (option == "--shader-dir") {
			if (++i == argc)
				throw std::invalid_argument(option + " requires a path");
			result.shaders = argv[i];
		} else if (option == "--help") {
			result.help = true;
			break;
		} else
			throw std::invalid_argument("Unknown argument: " + option);
	}

	return result;
}

void printControls(std::ostream &output) {
	output << "Controls: Enter deploy | WASD move | Z/X turret left/right | "
	          "Click/Space fire | P pause | R restart\n"
	          "RMB orbit | Wheel zoom | C reset camera | O projection | "
	          "L rotate light\n"
	          "F1 print controls | F3 toggle technique (shadow mapping) | Esc quit\n";
}

void printUsage(std::ostream &output) {
	output << "Isometric Tanks\n"
	          "  tanks [--shader-dir PATH]\n\n";

	printControls(output);
}
} // namespace tanks::app
