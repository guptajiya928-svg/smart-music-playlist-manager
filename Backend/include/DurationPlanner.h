
#pragma once

#include "../nlohmann/json.hpp"

using json = nlohmann::json;

// Generates a playlist within the target duration
json generateDurationPlaylist(
    const json& songs,
    int target
);
