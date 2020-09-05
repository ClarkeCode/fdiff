#include "../headers/FileRepresentation.hpp"

std::stringstream frep::FileRepFormatter::ss = std::stringstream();

std::string frep::FileRepFormatter::yieldReportString(FileRep::FRepCollection const& collection, ExecutionFlags const& flags, std::string columnSeparator, char horizontalRuleChar) {
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
