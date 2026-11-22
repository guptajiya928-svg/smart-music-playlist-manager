
#pragma once

#include <string>
using namespace std;

// Stores information about a song
struct Song {
    int id;
    string title;
    string artist;
    string mood;
    int duration;
};

// Node of the playlist linked list
struct Node {
    Song song;
    Node* next;

    Node(const Song& s) : song(s), next(nullptr) {}
};
