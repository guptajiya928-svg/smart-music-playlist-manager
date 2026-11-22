#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "httplib.h"
#include "ApiRoutes.h"
#include "Playlist.h"

#include <iostream>

int main() {
    Playlist playlist;
    playlist.reclassifyExistingSongs();
    httplib::Server server;

    server.Get("/api/health",
        [](const httplib::Request&, httplib::Response& res) {
            json result = {
                {"status", "ok"},
                {"message", "C++ backend is running"}
            };

            res.set_content(result.dump(), "application/json");
        });

    server.Get("/api/songs",
        [&playlist](const httplib::Request& req, httplib::Response& res) {
            res.set_content(
                playlist.getSongs().dump(),
                "application/json"
            );
        });
      
server.Get("/api/shuffle", [&](const httplib::Request& req,
                               httplib::Response& res) {
    json songs = playlist.getSongs();
    int n = static_cast<int>(songs.size());

    if (n == 0) {
        res.set_content("[]", "application/json");
        return;
    }

    // Step 1: Create a weighted graph of songs.
    vector<vector<pair<int, int>>> graph(n);

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            int score = 0;

            // Same mood gets the highest priority.
if (songs[i]["mood"] == songs[j]["mood"]) {
    score += 4;
}

// Same artist gets additional priority.
if (songs[i]["artist"] == songs[j]["artist"]) {
    score += 2;
}

            // Add an edge when the songs are related.
            if (score > 0) {
                graph[i].push_back({j, score});
                graph[j].push_back({i, score});
            }
        }
    }

    // Step 2: Visit songs using greedy selection.
    vector<bool> visited(n, false);
    json result = json::array();
    
int current = 0;

if (req.has_param("startId")) {

    string startId = req.get_param_value("startId");
    bool found = false;

    for (int i = 0; i < n; i++) {

        if (to_string(songs[i]["id"].get<int>()) == startId) {
            current = i;
            found = true;
            break;
        }
    }

    if (!found) {
        res.status = 404;
        res.set_content(
            R"({"error":"Selected song not found"})",
            "application/json"
        );
        return;
    }
}


    for (int count = 0; count < n; count++) {
        result.push_back(songs[current]);
        visited[current] = true;

        int next = -1;
        int bestScore = -1;

        // Find the most similar unvisited neighbour.
        for (const auto& edge : graph[current]) {
            int neighbour = edge.first;
            int score = edge.second;

            if (!visited[neighbour] && score > bestScore) {
                bestScore = score;
                next = neighbour;
            }
        }

        // If no related song remains, choose the
        // first song that has not been visited.
        if (next == -1) {
            for (int i = 0; i < n; i++) {
                if (!visited[i]) {
                    next = i;
                    break;
                }
            }
        }

        if (next == -1) {
            break;
        }

        current = next;
    }

    res.set_content(result.dump(), "application/json");
});

server.Get("/api/mood", [&](const httplib::Request& req,
                            httplib::Response& res) {
    if (!req.has_param("mood")) {
        res.status = 400;
        res.set_content(
            R"({"error":"Please provide a mood"})",
            "application/json"
        );
        return;
    }

    string wantedMood = req.get_param_value("mood");

    transform(wantedMood.begin(), wantedMood.end(),
              wantedMood.begin(),
              [](unsigned char c) {
                  return static_cast<char>(tolower(c));
              });

    json songs = playlist.getSongs();
    priority_queue<pair<int, int>> pq;

    // Matching mood gets priority 1.
    // Non-matching songs get priority 0.
    for (int i = 0; i < static_cast<int>(songs.size()); i++) {
        string songMood = songs[i]["mood"].get<string>();

        transform(songMood.begin(), songMood.end(),
                  songMood.begin(),
                  [](unsigned char c) {
                      return static_cast<char>(tolower(c));
                  });

        int score = (songMood == wantedMood) ? 1 : 0;

        // Negative index preserves earlier songs first
        // when their scores are equal.
        pq.push({score, -i});
    }

    json result = json::array();

    while (!pq.empty()) {
        int index = -pq.top().second;
        pq.pop();
        result.push_back(songs[index]);
    }

    res.set_content(result.dump(), "application/json");
});

server.Get("/api/duration", [&](const httplib::Request& req,
                               httplib::Response& res) {
    if (!req.has_param("seconds")) {
        res.status = 400;
        res.set_content(
            R"({"error":"Provide target duration in seconds"})",
            "application/json"
        );
        return;
    }

    int target = 0;

    try {
        target = stoi(req.get_param_value("seconds"));
    } catch (...) {
        res.status = 400;
        res.set_content(
            R"({"error":"Invalid duration"})",
            "application/json"
        );
        return;
    }

    // Limit target to 4 hours to keep memory usage controlled.
    if (target <= 0 || target > 14400) {
        res.status = 400;
        res.set_content(
            R"({"error":"Duration must be between 1 and 240 minutes"})",
            "application/json"
        );
        return;
    }

    json songs = playlist.getSongs();
    int n = static_cast<int>(songs.size());

    // dp[i][t] = maximum listening time possible
    // using the first i songs within t seconds.
    vector<vector<int>> dp(
        n + 1, vector<int>(target + 1, 0)
    );

    for (int i = 1; i <= n; i++) {
        int duration = songs[i - 1]["duration"].get<int>();

        for (int t = 0; t <= target; t++) {
            // Option 1: do not select this song.
            dp[i][t] = dp[i - 1][t];

            // Option 2: select this song if it fits.
            if (duration <= t) {
                int selected =
                    dp[i - 1][t - duration] + duration;

                if (selected > dp[i][t]) {
                    dp[i][t] = selected;
                }
            }
        }
    }

    // Reconstruct which songs were selected.
    json result = json::array();
    int remaining = target;

    for (int i = n; i >= 1; i--) {
        if (dp[i][remaining] != dp[i - 1][remaining]) {
            result.push_back(songs[i - 1]);

            int duration =
                songs[i - 1]["duration"].get<int>();

            remaining -= duration;
        }
    }

    // Reverse selection order to keep the original order.
    json ordered = json::array();

    for (int i = static_cast<int>(result.size()) - 1;
         i >= 0; i--) {
        ordered.push_back(result[i]);
    }

    res.set_content(ordered.dump(), "application/json");
});


    // Export playlist songs as a CSV file.
    server.Get("/api/export/csv", [&](const httplib::Request& req,
                                      httplib::Response& res) {
        json songs = playlist.getSongs();

        string csv = "ID,Title,Artist,Mood,Duration (seconds)\n";

        // Escape commas, quotes, and newlines in CSV values.
        auto escapeCSV = [](string value) {
            string escaped = "\"";

            for (char ch : value) {
                if (ch == '"') {
                    escaped += "\"\"";
                } else {
                    escaped += ch;
                }
            }

            escaped += "\"";
            return escaped;
        };

        for (const auto& song : songs) {
            csv += to_string(song["id"].get<int>()) + ",";
            csv += escapeCSV(song["title"].get<string>()) + ",";
            csv += escapeCSV(song["artist"].get<string>()) + ",";
            csv += escapeCSV(song["mood"].get<string>()) + ",";
            csv += to_string(song["duration"].get<int>()) + "\n";
        }

        res.set_header(
            "Content-Disposition",
            "attachment; filename=\"smart_music_playlist.csv\""
        );

        res.set_content(csv, "text/csv; charset=utf-8");
    });


    // Undo the most recent song deletion.
    server.Post("/api/undo", [&](const httplib::Request& req,
                                 httplib::Response& res) {
        if (playlist.undoDelete()) {
            res.set_content(
                R"({"message":"Undo successful"})",
                "application/json"
            );
        } else {
            res.status = 400;
            res.set_content(
                R"({"error":"Nothing to undo"})",
                "application/json"
            );
        }
    });

    // Redo the most recently undone deletion.
    server.Post("/api/redo", [&](const httplib::Request& req, 
                                 httplib::Response& res) {
        if (playlist.redoDelete()) {
            res.set_content(
                R"({"message":"Redo successful"})",
                "application/json"
            );
        } else {
            res.status = 400;
            res.set_content(
                R"({"error":"Nothing to redo"})",
                "application/json"
            );
        }
    });

    
server.Post("/api/songs",
    [&playlist](const httplib::Request& req,
                httplib::Response& res) {
        try {
            json body = json::parse(req.body);

            string title = body.at("title").get<string>();
            string artist = body.at("artist").get<string>();
            string selectedMood = body.at("mood").get<string>();
            int duration = body.at("duration").get<int>();

            if (title.empty() || artist.empty() || duration <= 0) {
                res.status = 400;
                res.set_content(
                    R"({"error":"Invalid song details"})",
                    "application/json"
                );
                return;
            }

            if (playlist.isDuplicate(title, artist)) {
                res.status = 409;
                res.set_content(
                    R"({"error":"Song already exists"})",
                    "application/json"
                );
                return;
            }

            // Normalize old mood names
            if (selectedMood == "calm") {
                selectedMood = "relax";
            }
            else if (selectedMood == "energetic") {
                selectedMood = "workout";
            }

            // Check selected mood validity
            vector<string> validMoods = {
                "study", "relax", "workout",
                "happy", "romantic", "sad"
            };

            if (find(validMoods.begin(), validMoods.end(),
                     selectedMood) == validMoods.end()) {
                res.status = 400;
                res.set_content(
                    R"({"error":"Invalid mood selected"})",
                    "application/json"
                );
                return;
            }

            // Automatic classification first
            string mood = playlist.classifyMood(title, artist);

            // If song is unknown, use user's selected mood
            if (mood == "unclassified") {
                mood = selectedMood;
            }

            int id = playlist.addSong(
                title, artist, mood, duration
            );

            json result = {
                {"message", "Song added successfully"},
                {"id", id},
                {"mood", mood}
            };

            res.status = 201;
            res.set_content(result.dump(), "application/json");

        } catch (const exception&) {
            res.status = 400;
            res.set_content(
                R"({"error":"Invalid JSON or missing fields"})",
                "application/json"
            );
        }
    });


    server.Delete(R"(/api/songs/(\d+))",
        [&playlist](const httplib::Request& req,
                    httplib::Response& res) {
            int id = stoi(req.matches[1].str());

            if (playlist.deleteSong(id)) {
                res.set_content(
                    R"({"message":"Song deleted successfully"})",
                    "application/json"
                );
            } else {
                res.status = 404;
                res.set_content(
                    R"({"error":"Song not found"})",
                    "application/json"
                );
            }
        });

    // Serve the frontend and API from the same local server.
    if (!server.set_mount_point("/", "../frontend"))  {
        cerr << "Could not find the frontend folder." << endl;
        return 1;
    }

    cout << "Smart Music backend starting..." << endl;
    cout << "Open http://localhost:8080" << endl;

    if (!server.listen("127.0.0.1", 8080)) {
        cerr << "Server could not start. Check whether port 8080 is busy."
             << endl;
        return 1;
    }

    return 0;
}
