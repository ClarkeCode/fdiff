#include <string>
#include <iostream>
#include <list>
#include <filesystem>
#include <vector>

#include <string>
#include <sstream>
#include <iomanip>

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
		FileRep(targetIt, shortFilePath, fileLocation, (targetIt->file_size() - masterIt->file_size())) {}

	//TODO: replace this with a format string or move funtionality to dedicated formatting class
	std::string yieldReportString() const {
		std::stringstream ss;
		ss << (fileLocation == InTarget ? '+' : (fileLocation == InMaster ? '-' : '~'));
		ss << " " << std::left << std::setw(10) << fullFileName;
		ss << std::right << std::setw(10) << fileSize;
		ss << std::right << std::setw(10);  if (fileLocation == InBoth) ss << fileSizeDifference; else ss << '-';
		return ss.str();
	}
};

class DirectoryHelper {
	std::string basePathString;
	path basePath;
	FileComparativeLocation directoryLocation;
	using diterator_t = recursive_directory_iterator;
	diterator_t fileIterator, endIterator;

	std::string stripLeadingSubstringFromPath(diterator_t rdi, std::string const& leadingSubstring) const {
		if (rdi == diterator_t()) return "";
		std::string pathStr(rdi->path().string());
		std::pair<std::string::const_iterator, std::string::const_iterator> ptrs = std::mismatch(pathStr.cbegin(), pathStr.cend(), leadingSubstring.begin(), leadingSubstring.end());
		return std::string(ptrs.first, pathStr.cend());
	}

public:
	DirectoryHelper(std::string const& input, FileComparativeLocation masterOrTarget) : directoryLocation(masterOrTarget) {
		basePathString = input;
		if (*basePathString.rbegin() != std::filesystem::path::preferred_separator)
			basePathString += std::filesystem::path::preferred_separator;

		basePath = path(basePathString);
		fileIterator = diterator_t(basePath);
	}

	std::string getFileShortName() const { return stripLeadingSubstringFromPath(fileIterator, basePathString); }
	bool isAtDirectoryEnd() const { return fileIterator == endIterator; }

	FileRep yieldCurrentFileReport() {
		FileRep output(fileIterator, getFileShortName(), directoryLocation);
		++fileIterator;
		return output;
	}

	FileRep yieldComparativeFileReport(DirectoryHelper& otherDirectoryHelper) {
		FileRep output(fileIterator, otherDirectoryHelper.fileIterator, getFileShortName(), FileComparativeLocation::InBoth);
		++fileIterator;
		++otherDirectoryHelper.fileIterator;
		return output;
	}
};

int main(int argc, char* argv[]) {
	if (argc == 1) show_usage();
	std::list<std::string> cmd_args;
	for (size_t x = 1; x < (size_t)argc; x++) {
		cmd_args.push_back(argv[x]);
	}

	DirectoryHelper masterDir(cmd_args.front(), FileComparativeLocation::InMaster);
	cmd_args.pop_front();

	DirectoryHelper targetDir(cmd_args.front(), FileComparativeLocation::InTarget);
	cmd_args.pop_front();
	
	ExeFlags exf{ 0 };

	FileRep::FRepCollection scanResults;

	do {
		//Condition: file in master is not found in targets
		if (targetDir.isAtDirectoryEnd() || masterDir.getFileShortName() < targetDir.getFileShortName()) {
			scanResults.push_back(masterDir.yieldCurrentFileReport());
		}

		//Condition: file in target did not exist in master
		else if (masterDir.isAtDirectoryEnd() || targetDir.getFileShortName() < masterDir.getFileShortName()) {
			scanResults.push_back(targetDir.yieldCurrentFileReport());
		}

		//Condition: file is in both master and target
		else if (masterDir.getFileShortName() == targetDir.getFileShortName()) {
			//Check for size difference
			FileRep foundFile(masterDir.yieldComparativeFileReport(targetDir));

			if (foundFile.fileSizeDifference != 0)
				scanResults.push_back(foundFile);
		}
	} while (!(masterDir.isAtDirectoryEnd() && targetDir.isAtDirectoryEnd()));
	

#define printout(item) std::cout << item << std::endl
	for (FileRep const& frep : scanResults) {
		printout(frep.yieldReportString());
	}
}