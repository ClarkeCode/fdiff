#pragma once
#include <filesystem>
#include <string>
#include <sstream>
#include "ExecutionFlags.hpp"

namespace frep {
	enum FileComparativeLocation { InMaster, InBoth, InTarget };

	struct FileRep {
		using FRepCollection = std::vector<FileRep>;
		using rdi = std::filesystem::recursive_directory_iterator;
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
		static inline std::string colHeaderFFN = "File Name";
		static inline std::string colHeaderFS = "File Size";
		static inline std::string colHeaderFSD = "Difference";
	public:
		static std::string yieldReportString(FileRep::FRepCollection const& collection, ExecutionFlags const& flags, std::string columnSeparator = " ", char horizontalRuleChar = '-');
	};
	
}