#pragma once

#include <string>

struct Song {
    int id;
    std::string title;
    std::string artist;
    std::string mood;
    int duration; // Duration in seconds
};
