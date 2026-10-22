#include "Playlist.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <utility>

using json = nlohmann::json;

Playlist::Playlist() : head(nullptr), nextId(1) {
    loadSongs();
}

Playlist::~Playlist() {
    while (head != nullptr) {
        Node* temp = head;
        head = head->next;
        delete temp;
    }
}

std::string Playlist::normalize(std::string text) const {
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
    return text;
}

void Playlist::saveSongs() const {
    std::ofstream file("songs.json");
    if (file.is_open()) {
        file << getSongs().dump(4);
        file.close();
    } else {
        std::cerr << "Could not save songs." << std::endl;
    }
}

void Playlist::loadSongs() {
    std::ifstream file("songs.json");

    // On the first run, songs.json may not exist. That is normal.
    if (!file.is_open()) {
        return;
    }

    try {
        json savedSongs;
        file >> savedSongs;

        if (!savedSongs.is_array()) {
            return;
        }

        Node* tail = nullptr;
        for (const auto& item : savedSongs) {
            Song song;
            song.id = item.at("id").get<int>();
            song.title = item.at("title").get<std::string>();
            song.artist = item.at("artist").get<std::string>();
            song.mood = item.at("mood").get<std::string>();
            song.duration = item.at("duration").get<int>();

            Node* newNode = new Node(song);
            if (head == nullptr) {
                head = newNode;
            } else {
                tail->next = newNode;
            }
            tail = newNode;

            if (song.id >= nextId) {
                nextId = song.id + 1;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Could not load songs: " << e.what() << std::endl;
    }
}

bool Playlist::isDuplicate(const std::string& title,
                           const std::string& artist) const {
    Node* current = head;
    while (current != nullptr) {
        if (normalize(current->song.title) == normalize(title) &&
            normalize(current->song.artist) == normalize(artist)) {
            return true;
        }
        current = current->next;
    }
    return false;
}

int Playlist::addSong(std::string title, std::string artist,
                      std::string mood, int duration) {
    Song song{nextId++, std::move(title), std::move(artist),
              std::move(mood), duration};
    Node* newNode = new Node(song);

    if (head == nullptr) {
        head = newNode;
    } else {
        Node* current = head;
        while (current->next != nullptr) {
            current = current->next;
        }
        current->next = newNode;
    }

    saveSongs();
    return song.id;
}

bool Playlist::deleteSong(int id) {
    Node* current = head;
    Node* previous = nullptr;
    int position = 0;

    while (current != nullptr) {
        if (current->song.id == id) {
            HistoryEntry deleted{current->song, position};

            if (previous == nullptr) {
                head = current->next;
            } else {
                previous->next = current->next;
            }

            delete current;
            undoStack.push_back(deleted);
            redoStack.clear();
            saveSongs();
            return true;
        }

        previous = current;
        current = current->next;
        ++position;
    }

    return false;
}

bool Playlist::undoDelete() {
    if (undoStack.empty()) {
        return false;
    }

    HistoryEntry entry = undoStack.back();
    undoStack.pop_back();
    Node* newNode = new Node(entry.song);

    // Restore the song at its original position.
    if (entry.position == 0 || head == nullptr) {
        newNode->next = head;
        head = newNode;
    } else {
        Node* current = head;
        int position = 0;
        while (current->next != nullptr && position < entry.position - 1) {
            current = current->next;
            ++position;
        }
        newNode->next = current->next;
        current->next = newNode;
    }

    redoStack.push_back(entry);
    saveSongs();
    return true;
}

bool Playlist::redoDelete() {
    if (redoStack.empty()) {
        return false;
    }

    HistoryEntry entry = redoStack.back();
    redoStack.pop_back();
    Node* current = head;
    Node* previous = nullptr;

    while (current != nullptr && current->song.id != entry.song.id) {
        previous = current;
        current = current->next;
    }

    if (current == nullptr) {
        return false;
    }

    if (previous == nullptr) {
        head = current->next;
    } else {
        previous->next = current->next;
    }

    delete current;
    undoStack.push_back(entry);
    saveSongs();
    return true;
}

Playlist::json Playlist::getSongs() const {
    json result = json::array();
    Node* current = head;

    while (current != nullptr) {
        const Song& song = current->song;
        result.push_back({
            {"id", song.id},
            {"title", song.title},
            {"artist", song.artist},
            {"mood", song.mood},
            {"duration", song.duration}
        });
        current = current->next;
    }

    return result;
}
