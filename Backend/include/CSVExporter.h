
#pragma once

#include "../nlohmann/json.hpp"
#include <string>

using json = nlohmann::json;
using std::string;

// Converts songs into CSV format
string exportSongsToCSV(const json& songs);
