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
    httplib::Server server;

    registerApiRoutes(server, playlist);

    // Serve the frontend and API from the same local server.
    if (!server.set_mount_point("/", "./frontend")) {
        std::cerr << "Could not find the frontend folder." << std::endl;
        return 1;
    }

    std::cout << "Smart Music backend starting..." << std::endl;
    std::cout << "Open http://localhost:8080" << std::endl;

    if (!server.listen("127.0.0.1", 8080)) {
        std::cerr << "Server could not start. Check whether port 8080 is busy."
                  << std::endl;
        return 1;
    }

    return 0;
}
