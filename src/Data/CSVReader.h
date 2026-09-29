#ifndef PRISM_CSV_READER_H
#define PRISM_CSV_READER_H

#include "Data/Dataset.h"
#include <string>

// ---------------------------------------------------------------------------
// CSVReader  -  loads a .csv file into a Dataset.
//
//  - First line = column names.
//  - Delimiter is detected automatically (comma, semicolon or tab).
//  - "Quoted, fields" are supported.
//  - Short rows are padded with empty (missing) cells; extra cells are ignored.
//  - After loading, DataValidator::inferTypes() decides numeric vs text.
// ---------------------------------------------------------------------------
class CSVReader {
public:
    static bool read(const std::string& path, Dataset& out, std::string& error);

    static char detectDelimiter(const std::string& headerLine);
    static DynamicArray<std::string> splitLine(const std::string& line, char delimiter);

    // Number of badly shaped rows found during the last read (for reporting).
    static int lastMalformedRows();

private:
    static int s_malformedRows;
};

#endif // PRISM_CSV_READER_H
