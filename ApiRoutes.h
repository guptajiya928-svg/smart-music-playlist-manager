#pragma once

#include "Playlist.h"

namespace httplib {
class Server;
}

void registerApiRoutes(httplib::Server& server, Playlist& playlist);
