#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "ApiRoutes.h"
#include "httplib.h"
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <queue>
#include <string>
#include <utility>
#include <vector>

using json = nlohmann::json;
using namespace std;

void registerApiRoutes(httplib::Server& server, Playlist& playlist) {
    server.Get("/api/health",
        [](const httplib::Request&, httplib::Response& res) {
            json result = {
                {"status", "ok"},
                {"message", "C++ backend is running"}
            };
            res.set_content(result.dump(), "application/json");
        });

    server.Get("/api/songs",
        [&playlist](const httplib::Request&, httplib::Response& res) {
            res.set_content(playlist.getSongs().dump(), "application/json");
        });

    server.Get("/api/shuffle",
        [&playlist](const httplib::Request&, httplib::Response& res) {
            json songs = playlist.getSongs();
            const int n = static_cast<int>(songs.size());

            if (n == 0) {
                res.set_content("[]", "application/json");
                return;
            }

            // Build a weighted graph: same artist = 4 points, same mood = 2.
            vector<vector<pair<int, int>>> graph(n);
            for (int i = 0; i < n; ++i) {
                for (int j = i + 1; j < n; ++j) {
                    int score = 0;
                    if (songs[i]["artist"] == songs[j]["artist"]) {
                        score += 4;
                    }
                    if (songs[i]["mood"] == songs[j]["mood"]) {
                        score += 2;
                    }
                    if (score > 0) {
                        graph[i].push_back({j, score});
                        graph[j].push_back({i, score});
                    }
                }
            }

            // Greedily pick the most similar song that has not been visited.
            vector<bool> visited(n, false);
            json result = json::array();
            int current = 0;

            for (int count = 0; count < n; ++count) {
                result.push_back(songs[current]);
                visited[current] = true;

                int next = -1;
                int bestScore = -1;
                for (const auto& edge : graph[current]) {
                    const int neighbour = edge.first;
                    const int score = edge.second;
                    if (!visited[neighbour] && score > bestScore) {
                        bestScore = score;
                        next = neighbour;
                    }
                }

                // If no related song remains, choose the first unvisited song.
                if (next == -1) {
                    for (int i = 0; i < n; ++i) {
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

    server.Get("/api/mood",
        [&playlist](const httplib::Request& req, httplib::Response& res) {
            if (!req.has_param("mood")) {
                res.status = 400;
                res.set_content(R"({"error":"Please provide a mood"})",
                                "application/json");
                return;
            }

            string wantedMood = req.get_param_value("mood");
            transform(wantedMood.begin(), wantedMood.end(), wantedMood.begin(),
                [](unsigned char c) {
                    return static_cast<char>(tolower(c));
                });

            json songs = playlist.getSongs();
            priority_queue<pair<int, int>> pq;

            for (int i = 0; i < static_cast<int>(songs.size()); ++i) {
                string songMood = songs[i]["mood"].get<string>();
                transform(songMood.begin(), songMood.end(), songMood.begin(),
                    [](unsigned char c) {
                        return static_cast<char>(tolower(c));
                    });

                const int score = (songMood == wantedMood) ? 1 : 0;
                // The negative index keeps earlier songs first for equal scores.
                pq.push({score, -i});
            }

            json result = json::array();
            while (!pq.empty()) {
                const int index = -pq.top().second;
                pq.pop();
                result.push_back(songs[index]);
            }

            res.set_content(result.dump(), "application/json");
        });

    server.Get("/api/duration",
        [&playlist](const httplib::Request& req, httplib::Response& res) {
            if (!req.has_param("seconds")) {
                res.status = 400;
                res.set_content(
                    R"({"error":"Provide target duration in seconds"})",
                    "application/json");
                return;
            }

            int target = 0;
            try {
                target = stoi(req.get_param_value("seconds"));
            } catch (...) {
                res.status = 400;
                res.set_content(R"({"error":"Invalid duration"})",
                                "application/json");
                return;
            }

            // Limit the target to four hours to control memory use.
            if (target <= 0 || target > 14400) {
                res.status = 400;
                res.set_content(
                    R"({"error":"Duration must be between 1 and 240 minutes"})",
                    "application/json");
                return;
            }

            json songs = playlist.getSongs();
            const int n = static_cast<int>(songs.size());
            // dp[i][t] = maximum total song duration not exceeding t,
            // using the first i songs. Each song can be selected at most once.
            vector<vector<int>> dp(n + 1, vector<int>(target + 1, 0));

            for (int i = 1; i <= n; ++i) {
                const int duration = songs[i - 1]["duration"].get<int>();
                for (int t = 0; t <= target; ++t) {
                    dp[i][t] = dp[i - 1][t];
                    if (duration <= t) {
                        const int selected = dp[i - 1][t - duration] + duration;
                        if (selected > dp[i][t]) {
                            dp[i][t] = selected;
                        }
                    }
                }
            }

            // Reconstruct the selected songs from the DP table.
            json result = json::array();
            int remaining = target;
            for (int i = n; i >= 1; --i) {
                if (dp[i][remaining] != dp[i - 1][remaining]) {
                    result.push_back(songs[i - 1]);
                    const int duration = songs[i - 1]["duration"].get<int>();
                    remaining -= duration;
                }
            }

            json ordered = json::array();
            for (int i = static_cast<int>(result.size()) - 1; i >= 0; --i) {
                ordered.push_back(result[i]);
            }
            res.set_content(ordered.dump(), "application/json");
        });

    // Export all songs as CSV.
    server.Get("/api/export/csv",
        [&playlist](const httplib::Request&, httplib::Response& res) {
            json songs = playlist.getSongs();
            string csv = "ID,Title,Artist,Mood,Duration (seconds)\n";

            auto escapeCSV = [](string value) {
                string escaped = "\"";
                for (char ch : value) {
                    if (ch == '\"') {
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

            res.set_header("Content-Disposition",
                           "attachment; filename=\"smart_music_playlist.csv\"");
            res.set_content(csv, "text/csv; charset=utf-8");
        });

    server.Post("/api/undo",
        [&playlist](const httplib::Request&, httplib::Response& res) {
            if (playlist.undoDelete()) {
                res.set_content(R"({"message":"Undo successful"})",
                                "application/json");
            } else {
                res.status = 400;
                res.set_content(R"({"error":"Nothing to undo"})",
                                "application/json");
            }
        });

    server.Post("/api/redo",
        [&playlist](const httplib::Request&, httplib::Response& res) {
            if (playlist.redoDelete()) {
                res.set_content(R"({"message":"Redo successful"})",
                                "application/json");
            } else {
                res.status = 400;
                res.set_content(R"({"error":"Nothing to redo"})",
                                "application/json");
            }
        });

    server.Post("/api/songs",
        [&playlist](const httplib::Request& req, httplib::Response& res) {
            try {
                json body = json::parse(req.body);
                string title = body.at("title").get<string>();
                string artist = body.at("artist").get<string>();
                string mood = body.at("mood").get<string>();
                int duration = body.at("duration").get<int>();

                if (title.empty() || artist.empty() || mood.empty() || duration <= 0) {
                    res.status = 400;
                    res.set_content(R"({"error":"Invalid song details"})",
                                    "application/json");
                    return;
                }

                if (playlist.isDuplicate(title, artist)) {
                    res.status = 409;
                    res.set_content(R"({"error":"Song already exists"})",
                                    "application/json");
                    return;
                }

                const int id = playlist.addSong(title, artist, mood, duration);
                json result = {
                    {"message", "Song added successfully"},
                    {"id", id}
                };
                res.status = 201;
                res.set_content(result.dump(), "application/json");
            } catch (const std::exception&) {
                res.status = 400;
                res.set_content(
                    R"({"error":"Invalid JSON or missing fields"})",
                    "application/json");
            }
        });

    server.Delete(R"(/api/songs/(\d+))",
        [&playlist](const httplib::Request& req, httplib::Response& res) {
            const int id = stoi(req.matches[1].str());
            if (playlist.deleteSong(id)) {
                res.set_content(R"({"message":"Song deleted successfully"})",
                                "application/json");
            } else {
                res.status = 404;
                res.set_content(R"({"error":"Song not found"})",
                                "application/json");
            }
        });
}
