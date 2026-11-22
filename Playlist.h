#pragma once

#include <string>
#include <vector>
#include <utility>

#include "Song.h"
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

class Playlist {

private:

    Node* head;
    int nextId;

    vector<pair<Song, int>> undoStack;
    vector<pair<Song, int>> redoStack;

    string normalize(string text) const;

    string getSongsFilePath() const;

    void saveSongs() const;

    void loadSongs();


public:

    Playlist();

    ~Playlist();

    string classifyMood(string title, string artist) const;

    void reclassifyExistingSongs();

    bool isDuplicate(string title, string artist) const;

    int addSong(string title, string artist,
                string mood, int duration);

    bool deleteSong(int id);

    bool undoDelete();

    bool redoDelete();

    json getSongs() const;
};
