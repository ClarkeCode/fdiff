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
//TODO: Implement option flags
struct ExeFlags {
	bool showFileNames;
	bool showFileSize;
	bool showFileSizeDifference;
	bool includeColumnSeparatorsOnEdges;
	bool includeHeaderHorizontalRule;
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
};

class FileRepFormatter {
	static std::stringstream ss;
	//static std::stringstream numeric;
	static inline std::string colHeaderFFN = "File Name";
	static inline std::string colHeaderFS  = "File Size";
	static inline std::string colHeaderFSD = "Difference";
public:
	static std::string yieldReportString(FileRep::FRepCollection const& collection, ExeFlags const& flags, std::string columnSeparator = " ", char horizontalRuleChar = '-') {
		FileRep::file_size_t colWidthFFN = colHeaderFFN.size(), colWidthFS = colHeaderFS.size(), colWidthFSD = colHeaderFSD.size();
		
		//Ensure fresh stringstream
		ss.str("");
		
		//Find maximum column width
		for (FileRep const& fr : collection) {
			size_t width = 0;
			if (flags.showFileNames) {
				ss << fr.fullFileName;
				width = ss.str().size();
				if (width > colWidthFFN) colWidthFFN = width;
				{ width = 0; ss.str(""); }
			}
			if (flags.showFileSize) {
				ss << fr.fileSize;
				width = ss.str().size();
				if (width > colWidthFS) colWidthFS = width;
				{ width = 0; ss.str(""); }
			}
			if (flags.showFileNames) {
				ss << fr.fileSizeDifference;
				width = ss.str().size();
				if (width > colWidthFSD) colWidthFSD = width;
				{ width = 0; ss.str(""); }
			}
		}

		//Compose the output

		//Generate the Header
#define newline(stream) stream << (flags.includeColumnSeparatorsOnEdges ? columnSeparator : "") << std::endl
		ss << " " << columnSeparator << std::left << std::setw(colWidthFFN) << colHeaderFFN;
		ss << columnSeparator << std::right << std::setw(colWidthFS) << colHeaderFS;
		ss << columnSeparator << std::right << std::setw(colWidthFSD) << colHeaderFSD;

		ss << (flags.includeColumnSeparatorsOnEdges ? columnSeparator : "");
		std::stringstream ssHeaderLength;
		ssHeaderLength << ss.rdbuf();
		ss << std::endl;
		//HR
		if (flags.includeHeaderHorizontalRule) {
			size_t whiteSpaceCorrectionAmount = columnSeparator.size() - columnSeparator.find_last_not_of(' ') - 1;
			ss << std::string(ssHeaderLength.str().size() - (flags.includeColumnSeparatorsOnEdges ? whiteSpaceCorrectionAmount : 0), horizontalRuleChar);
			ss << std::endl;
		}



		//Generate Lines
		for (FileRep const& fr : collection) {
			ss << (fr.fileLocation == InTarget ? '+' : (fr.fileLocation == InMaster ? '-' : '~'));
			if (flags.showFileNames) {
				ss << columnSeparator << std::left << std::setw(colWidthFFN) << fr.fullFileName;
			}
			if (flags.showFileSize) {
				ss << columnSeparator << std::right << std::setw(colWidthFS) << fr.fileSize;
			}
			if (flags.showFileNames) {
				ss << columnSeparator << std::right << std::setw(colWidthFSD);
				if (fr.fileLocation == InBoth) ss << fr.fileSizeDifference; else ss << '-';
			}

			newline(ss);
		}

		return ss.str();
	}
};
std::stringstream FileRepFormatter::ss = std::stringstream();

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
	
	ExeFlags exf{ 1, 1, 1, 1, 1 };

	FileRep::FRepCollection scanResults;

	do {
		//Condition: file in master is not found in targets
		if (!masterDir.isAtDirectoryEnd() && (targetDir.isAtDirectoryEnd() || masterDir.getFileShortName() < targetDir.getFileShortName())) {
			scanResults.push_back(masterDir.yieldCurrentFileReport());
		}

		//Condition: file in target did not exist in master
		else if (!targetDir.isAtDirectoryEnd() && (masterDir.isAtDirectoryEnd() || targetDir.getFileShortName() < masterDir.getFileShortName())) {
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
	printout(FileRepFormatter::yieldReportString(scanResults, exf, " | "));
}