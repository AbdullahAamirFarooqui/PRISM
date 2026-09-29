#ifndef PRISM_LOAD_MANAGER_H
#define PRISM_LOAD_MANAGER_H

#include "Data/Dataset.h"
#include "Data/HistoryLog.h"
#include <string>

// ---------------------------------------------------------------------------
// LoadManager - reads a .sda project file written by SaveManager.
// ---------------------------------------------------------------------------
class LoadManager {
public:
    static bool loadProject(const std::string& path, Dataset& data,
                            HistoryLog& history, std::string& error);
};

#endif // PRISM_LOAD_MANAGER_H
