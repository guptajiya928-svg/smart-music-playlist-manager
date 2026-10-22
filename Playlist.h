#pragma once

#include "Song.h"
#include <nlohmann/json.hpp>

#include <string>
#include <utility>
#include <vector>

class Playlist {
private:
    struct HistoryEntry {
        Song song;
        int position;
    };

    struct Node {
        Song song;
        Node* next;

        explicit Node(const Song& value) : song(value), next(nullptr) {}
    };

    Node* head;
    int nextId;
    std::vector<HistoryEntry> undoStack;
    std::vector<HistoryEntry> redoStack;

    std::string normalize(std::string text) const;
    void saveSongs() const;
    void loadSongs();

public:
    using json = nlohmann::json;

    Playlist();
    ~Playlist();

    Playlist(const Playlist&) = delete;
    Playlist& operator=(const Playlist&) = delete;

    bool isDuplicate(const std::string& title, const std::string& artist) const;
    int addSong(std::string title, std::string artist,
                std::string mood, int duration);
    bool deleteSong(int id);
    bool undoDelete();
    bool redoDelete();
    json getSongs() const;
};
