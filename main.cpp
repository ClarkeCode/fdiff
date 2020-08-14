#include <string>
#include <iostream>
#include <list>
#include <filesystem>

#include <algorithm> //mismatch
#include <utility> //pair

using namespace std::filesystem;

static void show_usage() {
	std::cerr << "Usage: fdiff <options> ORIGINAL TARGET" << std::endl;
}

struct ExeFlags {
	bool searchDeep;
};

struct FileRep {
	path filepath;
	std::string name, extension, fullname;
	uintmax_t filesize;
	bool inmaster, intarget;
	FileRep(recursive_directory_iterator it) : filepath(it->path()), name(it->path().stem().string()), extension(it->path().extension().string()),
		fullname(it->path().filename().string()), filesize(it->file_size()) {};
};

std::string stripLeadingSubstringFromPath(recursive_directory_iterator rdi, std::string const& leadingSubstring) {
	std::string pathStr(rdi->path().string());
	std::pair<std::string::const_iterator, std::string::const_iterator> ptrs = std::mismatch(pathStr.cbegin(), pathStr.cend(), leadingSubstring.begin(), leadingSubstring.end());
	return std::string(ptrs.first, pathStr.cend());
}

int main(int argc, char* argv[]) {
	if (argc == 1) show_usage();
	std::list<std::string> cmd_args;
	for (size_t x = 1; x < (size_t)argc; x++) {
		cmd_args.push_back(argv[x]);
	}

	std::string masterDirString(cmd_args.front());
	if (*masterDirString.rbegin() != '\\') { masterDirString += '\\'; }
	path masterDirPath(masterDirString);
	cmd_args.pop_front();

	std::string targetDirString(cmd_args.front());
	if (*targetDirString.rbegin() != '\\') { targetDirString += '\\'; }
	path targetDirPath(targetDirString);
	cmd_args.pop_front();
	
	recursive_directory_iterator masterDir(masterDirPath);
	recursive_directory_iterator targetDir(targetDirPath);
	ExeFlags exf{ 0 };

#define printout(item) std::cout << item << std::endl

	while (masterDir != recursive_directory_iterator()) {
		printout(masterDir->path().string() << " - " << masterDir->file_size());

		printout((masterDir->is_directory() ? "directory" : "file"));
		printout(stripLeadingSubstringFromPath(masterDir, masterDirString));
		printout("");
		masterDir++;
	}
	printout("");
	while (targetDir != recursive_directory_iterator()) {
		printout(targetDir->path().string() << " - " << targetDir->file_size());

		printout((targetDir->is_directory() ? "directory" : "file"));
		printout(stripLeadingSubstringFromPath(targetDir, targetDirString));
		printout("");
		targetDir++;
	}
}