#include <string>
#include <iostream>
#include <list>
#include <filesystem>
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
	FileRep(recursive_directory_iterator it) : filepath(it->path()), name(it->path().stem().string()), extension(it->path().extension().string()),
		fullname(it->path().filename().string()), filesize(it->file_size()) {};
};

int main(int argc, char* argv[]) {
	if (argc == 1) show_usage();
	std::list<std::string> cmd_args;
	for (size_t x = 1; x < (size_t)argc; x++) {
		cmd_args.push_back(argv[x]);
	}

	path masterDirPath(cmd_args.front());
	cmd_args.pop_front();
	path targetDirPath(cmd_args.front());
	
	recursive_directory_iterator masterDir(masterDirPath);
	recursive_directory_iterator targetDir(targetDirPath);
	ExeFlags a{ 0 };

	while (masterDir != recursive_directory_iterator()) {
		std::cout << masterDir->path().string() << " - " << masterDir->file_size() << std::endl;
		masterDir++;
	}
	while (targetDir != recursive_directory_iterator()) {
		std::cout << targetDir->path().string() << " - " << targetDir->file_size() << std::endl;
		targetDir++;
	}
}