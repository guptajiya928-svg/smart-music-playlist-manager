
#include "../include/SmartShuffle.h"
#include <vector>
#include <string>

using namespace std;

bool generateSmartShuffle(
    const json& songs,
    const string& startId,
    json& result
) {
    int n = static_cast<int>(songs.size());
    result = json::array();

    if (n == 0) return true;

    // Create a weighted similarity graph
    vector<vector<pair<int, int>>> graph(n);

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            int score = 0;

            if (songs[i]["mood"] == songs[j]["mood"])
                score += 4;

            if (songs[i]["artist"] == songs[j]["artist"])
                score += 2;

            if (score > 0) {
                graph[i].push_back({j, score});
                graph[j].push_back({i, score});
            }
        }
    }

    vector<bool> visited(n, false);
    int current = 0;

    // Start from the selected song, if provided
    if (!startId.empty()) {
        bool found = false;

        for (int i = 0; i < n; i++) {
            if (to_string(songs[i]["id"].get<int>()) == startId) {
                current = i;
                found = true;
                break;
            }
        }

        if (!found) return false;
    }

    // Greedy traversal of the graph
    for (int count = 0; count < n; count++) {
        result.push_back(songs[current]);
        visited[current] = true;

        int next = -1;
        int bestScore = -1;

        for (const auto& edge : graph[current]) {
            int neighbour = edge.first;
            int score = edge.second;

            if (!visited[neighbour] && score > bestScore) {
                bestScore = score;
                next = neighbour;
            }
        }

        // If no connected song is available, select
        // the first unvisited song
        if (next == -1) {
            for (int i = 0; i < n; i++) {
                if (!visited[i]) {
                    next = i;
                    break;
                }
            }
        }

        if (next == -1) break;

        current = next;
    }

    return true;
}
