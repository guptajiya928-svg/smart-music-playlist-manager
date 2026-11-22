
#pragma once

#include "../nlohmann/json.hpp"
#include <string>

using json = nlohmann::json;
using std::string;

// Returns songs matching the selected mood
json getMoodRecommendations(
    const json& songs,
    const string& wantedMood
);
