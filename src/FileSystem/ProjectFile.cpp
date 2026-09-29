#include "FileSystem/ProjectFile.h"

const char* ProjectFile::MAGIC = "PRISM-SDA";

std::string ProjectFile::escape(const std::string& text) {
    std::string out;
    for (char ch : text) {
        switch (ch) {
        case '\\': out += "\\\\"; break;
        case '\t': out += "\\t"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        default:   out += ch;
        }
    }
    return out;
}

std::string ProjectFile::unescape(const std::string& text) {
    std::string out;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\\' && i + 1 < text.size()) {
            char next = text[++i];
            if (next == 't') out += '\t';
            else if (next == 'n') out += '\n';
            else if (next == 'r') out += '\r';
            else out += next;            // "\\" -> "\"
        } else {
            out += text[i];
        }
    }
    return out;
}

DynamicArray<std::string> ProjectFile::splitFields(const std::string& line) {
    DynamicArray<std::string> fields;
    std::string current;
    for (char ch : line) {
        if (ch == '\t') {
            fields.pushBack(unescape(current));
            current.clear();
        } else {
            current += ch;
        }
    }
    fields.pushBack(unescape(current));
    return fields;
}
