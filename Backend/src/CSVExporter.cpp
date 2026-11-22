
#include "../include/CSVExporter.h"

#include <string>

using namespace std;

// Handles commas and quotation marks in CSV values
string escapeCSV(const string& value) {
    string escaped = "\"";

    for (char ch : value) {
        if (ch == '"') {
            escaped += "\"\"";
        } else {
            escaped += ch;
        }
    }

    escaped += "\"";
    return escaped;
}

// Converts all songs into CSV format
string exportSongsToCSV(const json& songs) {
    string csv = "ID,Title,Artist,Mood,Duration (seconds)\n";

    for (const auto& song : songs) {
        csv += to_string(song["id"].get<int>()) + ",";
        csv += escapeCSV(song["title"].get<string>()) + ",";
        csv += escapeCSV(song["artist"].get<string>()) + ",";
        csv += escapeCSV(song["mood"].get<string>()) + ",";
        csv += to_string(song["duration"].get<int>()) + "\n";
    }

    return csv;
}
