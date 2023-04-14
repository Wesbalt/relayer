#include "client.h"
#include <iostream>

/*
 * Takes the server IP and port as arguments and starts the client
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::clog << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
        return 1;
    }
    auto server_ip   = argv[1];
    auto server_port = argv[2]; // TODO: Read this from server.properties
    return start_client(server_ip, server_port);
}
