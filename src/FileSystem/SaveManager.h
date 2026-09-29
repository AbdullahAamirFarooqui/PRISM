#ifndef PRISM_SAVE_MANAGER_H
#define PRISM_SAVE_MANAGER_H

#include "Data/Dataset.h"
#include "Data/HistoryLog.h"
#include <string>

// ---------------------------------------------------------------------------
// SaveManager - writes a session to disk.
//   saveProject : the custom .sda format (see ProjectFile.h)
//   exportCSV   : the (cleaned) dataset as a normal .csv file
// ---------------------------------------------------------------------------
class SaveManager {
public:
    static bool saveProject(const std::string& path, const Dataset& data,
                            const HistoryLog& history, std::string& error);

    static bool exportCSV(const std::string& path, const Dataset& data, std::string& error);

private:
    static std::string csvField(const std::string& text);   // adds quotes when needed
};

#endif // PRISM_SAVE_MANAGER_H
