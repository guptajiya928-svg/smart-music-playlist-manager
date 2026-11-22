
#include "../include/Playlist.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

// Automatically classify a song based on its title and artist
string Playlist::classifyMood(string title, string artist) const {

    string normalizedTitle = normalize(title);

    static const unordered_map<string, string> knownSongs = {

        // Study
        {"experience", "study"},
        {"rivers flows in you", "study"},
        {"nuvole bianche", "study"},

        // Relax
        {"weightless", "relax"},
        {"snowfall", "relax"},
        {"a moment apart", "relax"},
        {"baarish", "relax"},
        {"safarnama", "relax"},
        {"ek zindagi", "relax"},

        // Romantic
        {"tum ho", "romantic"},
        {"mann mera", "romantic"},
        {"iktara", "romantic"},
        {"kasoor", "romantic"},
        {"husn", "romantic"},
        {"softly", "romantic"},
        {"kesariya", "romantic"},
        {"raataan lambiyan", "romantic"},
        {"heeriye", "romantic"},
        {"apna bana le", "romantic"},
        {"phir bhi tumko chaahunga", "romantic"},
        {"all of me", "romantic"},
        {"a thousand years", "romantic"},
        {"shayad", "romantic"},
        {"o maahi", "romantic"},
        {"finding her", "romantic"},
        {"senorita", "romantic"},
        {"lover", "romantic"},
        {"tum hi hi", "romantic"},
        {"my love", "romantic"},

        // Sad
        {"the night we met", "sad"},
        {"channa mereya", "sad"},
        {"ranjha", "sad"},
        {"satranga", "sad"},
        {"hamari adhuri kahani", "sad"},
        {"tune jo na kaha", "sad"},
        {"bekhayali", "sad"},
        {"someone you loved", "sad"},
        {"let her go", "sad"},
        {"phir le aya dil", "sad"},
        {"baarishein", "sad"},
        {"excuses", "sad"},

        // Workout
        {"believer", "workout"},
        {"born to shine", "workout"},
        {"goat", "workout"},
        {"we rollin", "workout"},
        {"eye of the tiger", "workout"},
        {"survivor", "workout"},
        {"52 bars", "workout"},
        {"stronger", "workout"},
        {"can't hold us", "workout"},
        {"tauba tauba", "workout"},
        {"kar har maidaan fateh", "workout"},
        {"sultan", "workout"},
        {"dangal", "workout"},
        {"brown munde", "workout"},
        {"bhaag milkha bhaag", "workout"},
        {"ziddi", "workout"},

        // Happy
        {"ilahi", "happy"},
        {"love you zindagi", "happy"},
        {"gallan goodiyaan", "happy"},
        {"on top of the world", "happy"},
        {"happy", "happy"},
        {"can't stop the feeling!", "happy"},
        {"levitating", "happy"},
        {"dil dhadakne do", "happy"},
        {"nachde ne saare", "happy"},
        {"cheap thrills", "happy"},
        {"ghungroo", "happy"}
    };

    // First check the known song database
    auto it = knownSongs.find(normalizedTitle);

    if (it != knownSongs.end()) {
        return it->second;
    }

    // If song is unknown, use keyword classification
    string text = normalize(title + " " + artist);

    vector<pair<string, vector<string>>> moodKeywords = {

        {"study", {"study", "focus", "concentration"}},

        {"relax", {"relax", "calm", "peace", "sleep",
                   "meditation", "ambient"}},

        {"workout", {"workout", "gym", "running",
                     "fitness", "exercise", "pump"}},

        {"happy", {"happy", "joy", "smile",
                   "celebrate", "party", "dance"}},

        {"romantic", {"love", "lover", "romantic",
                      "ishq", "pyaar", "pyar",
                      "mohabbat", "jaan"}},

        {"sad", {"sad", "alone", "broken", "cry",
                 "tears", "dard", "bewafa",
                 "tanha", "udaas"}}
    };

    string bestMood = "unclassified";
    int highestScore = 0;
    bool tie = false;

    for (const auto& category : moodKeywords) {

        int score = 0;

        for (const string& keyword : category.second) {

            if (text.find(keyword) != string::npos) {
                score++;
            }
        }

        if (score > highestScore) {
            highestScore = score;
            bestMood = category.first;
            tie = false;
        }
        else if (score > 0 && score == highestScore) {
            tie = true;
        }
    }

    if (highestScore == 0 || tie) {
        return "unclassified";
    }

    return bestMood;
}


// Reclassify previously saved songs
void Playlist::reclassifyExistingSongs() {

    Node* current = head;

    while (current != nullptr) {

        string oldMood = current->song.mood;

        // Keep these moods unchanged
        if (oldMood == "happy" ||
            oldMood == "romantic" ||
            oldMood == "sad") {

            current = current->next;
            continue;
        }

        // Energetic becomes Workout
        if (oldMood == "energetic") {
            current->song.mood = "workout";
        }

        // Calm songs become Study or Relax
        else if (oldMood == "calm") {

            string detectedMood =
                classifyMood(current->song.title,
                             current->song.artist);

            if (detectedMood == "study" ||
                detectedMood == "relax") {

                current->song.mood = detectedMood;
            }
            else {
                current->song.mood = "relax";
            }
        }

        current = current->next;
    }

    saveSongs();

    cout << "Existing songs mood updated successfully!" << endl;
}
