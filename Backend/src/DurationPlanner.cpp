
#include "../include/DurationPlanner.h"

#include <vector>
#include <algorithm>

using namespace std;

json generateDurationPlaylist(
    const json& songs,
    int target
) {
    int n = static_cast<int>(songs.size());

    // dp[i][j] stores maximum songs' duration
    // possible using first i songs within j seconds
    vector<vector<int>> dp(
        n + 1,
        vector<int>(target + 1, 0)
    );

    for (int i = 1; i <= n; i++) {
        int duration = songs[i - 1]["duration"].get<int>();

        for (int j = 0; j <= target; j++) {
            dp[i][j] = dp[i - 1][j];

            if (duration <= j) {
                dp[i][j] = max(
                    dp[i][j],
                    duration + dp[i - 1][j - duration]
                );
            }
        }
    }

    // Reconstruct selected songs
    json result = json::array();
    int remaining = target;

    for (int i = n; i >= 1; i--) {
        if (dp[i][remaining] != dp[i - 1][remaining]) {
            result.push_back(songs[i - 1]);

            remaining -= songs[i - 1]["duration"].get<int>();
        }
    }

    // Maintain original playlist order
    reverse(result.begin(), result.end());

    return result;
}
