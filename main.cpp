#include <string>
#include <iostream>
#include <list>
#include <filesystem>
#include <vector>

#include <algorithm> //mismatch
#include <utility> //pair

using namespace std::filesystem;

static void show_usage() {
	std::cerr << "Usage: fdiff <options> ORIGINAL TARGET" << std::endl;
}

struct ExeFlags {
	bool searchDeep;
};

enum FileComparativeLocation { InMaster, InBoth, InTarget};

struct FileRep {
	using FRepCollection = std::vector<FileRep>;
	using rdi = recursive_directory_iterator;
	using file_size_t = uintmax_t;
	using file_size_diff_t = long long;

	std::string fileName, fileExtension, fullFileName;
	file_size_t fileSize;
	file_size_diff_t fileSizeDifference;
	bool isDirectory;
	FileComparativeLocation fileLocation;

	FileRep(rdi it, std::string shortFilePath, FileComparativeLocation fileLocation, file_size_diff_t sizeDifference = 0) : fullFileName(shortFilePath),
		fileName(it->path().stem().string()), fileExtension(it->path().extension().string()), isDirectory(it->is_directory()),
		fileSize(it->file_size()), fileLocation(fileLocation), fileSizeDifference(sizeDifference) {};

	FileRep(rdi masterIt, rdi targetIt, std::string shortFilePath, FileComparativeLocation fileLocation) : 
		FileRep(masterIt, shortFilePath, fileLocation, (targetIt->file_size() - masterIt->file_size())) {}
};

std::string stripLeadingSubstringFromPath(recursive_directory_iterator rdi, std::string const& leadingSubstring) {
	if (rdi == recursive_directory_iterator()) return "";
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

	FileRep::FRepCollection scanResults;

	std::string masterFileShortName = stripLeadingSubstringFromPath(masterDir, masterDirString);
	std::string targetFileShortName = stripLeadingSubstringFromPath(targetDir, targetDirString);

	do {
		//Condition: file in master is not found in targets
		if (targetDir == recursive_directory_iterator() || masterFileShortName < targetFileShortName) {
			scanResults.push_back(FileRep(masterDir, masterFileShortName, FileComparativeLocation::InMaster));
			++masterDir;
			masterFileShortName = stripLeadingSubstringFromPath(masterDir, masterDirString);
		}

		//Condition: file in target did not exist in master
		else if (masterDir == recursive_directory_iterator() || targetFileShortName < masterFileShortName) {
			scanResults.push_back(FileRep(targetDir, targetFileShortName, FileComparativeLocation::InTarget));
			++targetDir;
			targetFileShortName = stripLeadingSubstringFromPath(targetDir, targetDirString);
		}

		//Condition: file is in both master and target
		else if (masterFileShortName == targetFileShortName) {
			//Check for size difference
			FileRep foundFile(masterDir, targetDir, masterFileShortName, FileComparativeLocation::InBoth);
			++masterDir;
			++targetDir;
			masterFileShortName = stripLeadingSubstringFromPath(masterDir, masterDirString);
			targetFileShortName = stripLeadingSubstringFromPath(targetDir, targetDirString);

			if (foundFile.fileSizeDifference != 0)
				scanResults.push_back(foundFile);
		}
	} while (masterDir != recursive_directory_iterator() && targetDir != recursive_directory_iterator());
	printout("Done");
}