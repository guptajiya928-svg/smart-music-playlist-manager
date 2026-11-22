
#pragma once

#include "../nlohmann/json.hpp"
#include <string>
using std::string;

using json = nlohmann::json;

// Generates a smart shuffled playlist
// Returns false if the selected song ID is not found
bool generateSmartShuffle(
    const json& songs,
    const string& startId,
    json& result
);
