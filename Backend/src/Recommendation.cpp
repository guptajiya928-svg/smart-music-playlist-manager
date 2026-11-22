
#include "../include/Recommendation.h"

#include <queue>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

using namespace std;

json getMoodRecommendations(
    const json& songs,
    const string& wantedMood
) {
    string targetMood = wantedMood;

    transform(
        targetMood.begin(),
        targetMood.end(),
        targetMood.begin(),
        [](unsigned char c) {
            return static_cast<char>(tolower(c));
        }
    );

    priority_queue<pair<int, int>> pq;

    for (int i = 0; i < static_cast<int>(songs.size()); i++) {
        string songMood = songs[i]["mood"].get<string>();

        transform(
            songMood.begin(),
            songMood.end(),
            songMood.begin(),
            [](unsigned char c) {
                return static_cast<char>(tolower(c));
            }
        );

        int score = (songMood == targetMood) ? 1 : 0;

        // Matching songs get higher priority.
        // Negative index preserves original order.
        pq.push({score, -i});
    }

    json result = json::array();

    while (!pq.empty()) {
        int index = -pq.top().second;
        pq.pop();

        result.push_back(songs[index]);
    }

    return result;
}
