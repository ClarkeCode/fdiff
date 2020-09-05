#include <string>
#include <iostream>
#include <list>
#include <filesystem>
#include <vector>

#include <string>
#include <sstream>
#include <iomanip>


#include <utility> //pair

#include "source/headers/ExecutionFlags.hpp"
#include "source/headers/DirectoryHelper.hpp"
using namespace std::filesystem;

static void show_usage() {
	std::cerr << "Usage: fdiff <options> ORIGINAL TARGET" << std::endl;
}

int fdiff_processing(int argc, char* argv[]) {
	if (argc == 1) show_usage();
	std::list<std::string> cmd_args;
	for (size_t x = 1; x < (size_t)argc; x++) {
		cmd_args.push_back(argv[x]);
	}

	//TODO: Implement processing of option flags
	DirectoryHelper masterDir(cmd_args.front(), frep::FileComparativeLocation::InMaster);
	cmd_args.pop_front();

	DirectoryHelper targetDir(cmd_args.front(), frep::FileComparativeLocation::InTarget);
	cmd_args.pop_front();

	ExecutionFlags exf{ 1, 1, 1, 1, 1, 1 };

	frep::FileRep::FRepCollection scanResults;

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
			frep::FileRep foundFile(masterDir.yieldComparativeFileReport(targetDir));

			if (foundFile.fileSizeDifference != 0)
				scanResults.push_back(foundFile);
		}
	} while (!(masterDir.isAtDirectoryEnd() && targetDir.isAtDirectoryEnd()));


#define printout(item) std::cout << item << std::endl
	printout(frep::FileRepFormatter::yieldReportString(scanResults, exf, " | "));

	return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
	return fdiff_processing(argc, argv);
}