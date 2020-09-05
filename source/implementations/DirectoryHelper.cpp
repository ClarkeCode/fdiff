#include "..\headers\DirectoryHelper.hpp"
#include <algorithm> //mismatch

std::string DirectoryHelper::stripLeadingSubstringFromPath(diterator_t rdi, std::string const& leadingSubstring) const {
	if (rdi == diterator_t()) return "";
	std::string pathStr(rdi->path().string());
	std::pair<std::string::const_iterator, std::string::const_iterator> ptrs = std::mismatch(pathStr.cbegin(), pathStr.cend(), leadingSubstring.begin(), leadingSubstring.end());
	return std::string(ptrs.first, pathStr.cend());
}

frep::FileRep DirectoryHelper::yieldCurrentFileReport() {
	frep::FileRep output(fileIterator, getFileShortName(), directoryLocation);
	++fileIterator;
	return output;
}

frep::FileRep DirectoryHelper::yieldComparativeFileReport(DirectoryHelper& otherDirectoryHelper) {
	frep::FileRep output(fileIterator, otherDirectoryHelper.fileIterator, getFileShortName(), frep::FileComparativeLocation::InBoth);
	++fileIterator;
	++otherDirectoryHelper.fileIterator;
	return output;
}
