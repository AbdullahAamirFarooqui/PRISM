#ifndef PRISM_PROJECT_FILE_H
#define PRISM_PROJECT_FILE_H

#include "DSA/DynamicArray.h"
#include <string>

// ---------------------------------------------------------------------------
// ProjectFile - describes the custom ".sda" (Scientific Data Analyzer)
// project format and small helpers shared by SaveManager and LoadManager.
//
// It is a plain text file, one record per line, fields separated by TAB:
//
//   PRISM-SDA   1
//   NAME        weather
//   SOURCE      C:/data/weather.csv
//   COLUMNS     3
//   COLUMN      N   Temperature          (N = numeric, T = text)
//   COLUMN      T   City
//   ...
//   ROWS        365
//   25.1   Karachi   ...                 (one line per data row)
//   HISTORY     2
//   2026-09-29 10:00:00   Imported 'weather'
//   END
//
// Tabs, new lines and backslashes inside a cell are escaped as \t \n \\ .
// ---------------------------------------------------------------------------
class ProjectFile {
public:
    static const char* MAGIC;          // "PRISM-SDA"
    static const int VERSION = 1;

    static std::string escape(const std::string& text);
    static std::string unescape(const std::string& text);

    // Splits a line on TAB characters (then unescapes every field).
    static DynamicArray<std::string> splitFields(const std::string& line);
};

#endif // PRISM_PROJECT_FILE_H
