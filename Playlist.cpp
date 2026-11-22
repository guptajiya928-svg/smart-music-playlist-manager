#include "../include/Playlist.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <exception>

using namespace std;

// Convert text into lowercase
string Playlist::normalize(string text) const {
    transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) {
            return static_cast<char>(tolower(c));
        });

    return text;
}

// Find the location of songs.json
string Playlist::getSongsFilePath() const {

    ifstream currentFile("songs.json");

    if (currentFile.is_open()) {
        currentFile.close();
        return "songs.json";
    }

    ifstream parentFile("../songs.json");

    if (parentFile.is_open()) {
        parentFile.close();
        return "../songs.json";
    }

    return "songs.json";
}

// Save playlist data into JSON file
void Playlist::saveSongs() const {

    ofstream file(getSongsFilePath());

    if (file.is_open()) {
        file << getSongs().dump(4);
        file.close();
    }
    else {
        cerr << "Could not save songs." << endl;
    }
}

// Load previously saved songs
void Playlist::loadSongs() {

    ifstream file(getSongsFilePath());

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

            Song s;

            s.id = item.at("id").get<int>();
            s.title = item.at("title").get<string>();
            s.artist = item.at("artist").get<string>();
            s.mood = item.at("mood").get<string>();
            s.duration = item.at("duration").get<int>();

            Node* newNode = new Node(s);

            if (head == nullptr) {
                head = newNode;
            }
            else {
                tail->next = newNode;
            }

            tail = newNode;

            if (s.id >= nextId) {
                nextId = s.id + 1;
            }
        }
    }
    catch (const exception& e) {
        cerr << "Could not load songs: "
             << e.what() << endl;
    }
}

// Constructor
Playlist::Playlist() : head(nullptr), nextId(1) {
    loadSongs();
}

// Destructor
Playlist::~Playlist() {

    while (head != nullptr) {
        Node* temp = head;
        head = head->next;
        delete temp;
    }
}

// Check whether a song already exists
bool Playlist::isDuplicate(string title, string artist) const {

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

// Add a new song
int Playlist::addSong(string title, string artist,
                      string mood, int duration) {

    Song s = {nextId++, title, artist, mood, duration};

    Node* newNode = new Node(s);

    if (head == nullptr) {
        head = newNode;
    }
    else {
        Node* current = head;

        while (current->next != nullptr) {
            current = current->next;
        }

        current->next = newNode;
    }
    
    redoStack.clear();  
    saveSongs();

    return s.id;
}

// Delete a song
bool Playlist::deleteSong(int id) {

    Node* current = head;
    Node* previous = nullptr;
    int position = 0;

    while (current != nullptr) {

        if (current->song.id == id) {

            pair<Song, int> deleted = {
                current->song, position
            };

            if (previous == nullptr) {
                head = current->next;
            }
            else {
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
        position++;
    }

    return false;
}

// Undo the most recent deletion
bool Playlist::undoDelete() {

    if (undoStack.empty()) {
        return false;
    }

    pair<Song, int> entry = undoStack.back();
    undoStack.pop_back();

    Node* newNode = new Node(entry.first);

    if (entry.second == 0 || head == nullptr) {

        newNode->next = head;
        head = newNode;
    }
    else {

        Node* current = head;
        int position = 0;

        while (current->next != nullptr &&
               position < entry.second - 1) {

            current = current->next;
            position++;
        }

        newNode->next = current->next;
        current->next = newNode;
    }

    redoStack.push_back(entry);

    saveSongs();

    return true;
}

// Redo the most recently undone deletion
bool Playlist::redoDelete() {

    if (redoStack.empty()) {
        return false;
    }

    pair<Song, int> entry = redoStack.back();
    redoStack.pop_back();

    Node* current = head;
    Node* previous = nullptr;

    while (current != nullptr &&
           current->song.id != entry.first.id) {

        previous = current;
        current = current->next;
    }

    if (current == nullptr) {
        return false;
    }

    if (previous == nullptr) {
        head = current->next;
    }
    else {
        previous->next = current->next;
    }

    delete current;

    undoStack.push_back(entry);

    saveSongs();

    return true;
}

// Return all songs in JSON format
json Playlist::getSongs() const {

    json result = json::array();

    Node* current = head;

    while (current != nullptr) {

        const Song& s = current->song;

        result.push_back({
            {"id", s.id},
            {"title", s.title},
            {"artist", s.artist},
            {"mood", s.mood},
            {"duration", s.duration}
        });

        current = current->next;
    }

    return result;
}
