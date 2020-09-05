#pragma once
#include <filesystem>
#include <vector>
#include "FileRepresentation.hpp"

class DirectoryHelper {
	std::string basePathString;
	std::filesystem::path basePath;
	frep::FileComparativeLocation directoryLocation;
	using diterator_t = std::filesystem::recursive_directory_iterator;
	diterator_t fileIterator, endIterator;

	std::string stripLeadingSubstringFromPath(diterator_t rdi, std::string const& leadingSubstring) const;

public:
	DirectoryHelper(std::string const& input, frep::FileComparativeLocation masterOrTarget) : directoryLocation(masterOrTarget) {
		basePathString = input;
		if (*basePathString.rbegin() != std::filesystem::path::preferred_separator)
			basePathString += std::filesystem::path::preferred_separator;

		basePath = std::filesystem::path(basePathString);
		fileIterator = diterator_t(basePath);
	}

	inline std::string getFileShortName() const { return stripLeadingSubstringFromPath(fileIterator, basePathString); }
	inline bool isAtDirectoryEnd() const { return fileIterator == endIterator; }

	frep::FileRep yieldCurrentFileReport();

	frep::FileRep yieldComparativeFileReport(DirectoryHelper& otherDirectoryHelper);
};