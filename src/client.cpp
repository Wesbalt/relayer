#include "client.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <cmath>
#include <thread>
#include <iomanip>
#include <mutex>

// The size of the client's receive data buffer
const u_int RECEIVE_BUFFER_LENGTH = 1024;

// The duration between info relays
const auto RELAY_INTERVAL = std::chrono::milliseconds(1500);

// The global list of objects that the client has received.
// We could use a hashtable but they aren't worth the
// overhead for a small number of elements.
std::vector<Object> objects;

std::mutex objects_mutex;

/*
 * Split the string at every occurrence of the separator. The separator
 * is removed at every split. Empty strings are permitted, for example:
 *
 *     split_string(" Many   spaces ", ' ') -> ["", "Many", "", "", "spaces", ""]
 */
std::vector<std::string> split_string(const std::string str, const char sep) {

    std::vector<std::string> substrings;
    int start = 0; // Index of the last split, i.e., the start of the next substring

    for (std::size_t i = 0; i <= str.length(); i++) {
        if (i >= str.length() || str[i] == sep) {
            // New substring found, either due to reaching the end of
            // the string or finding the separator
            const std::string substring = str.substr(start, i-start);
            substrings.push_back(substring);
            start = i+1; // Add one to skip the separator
        }
    }
    return substrings;
}

/*
 * Parse the string as an Object based on this format:
 *
 *     ID=572912;X=50;Y=130;TYPE=1
 *
 * The ID is a 64-bit int, X and Y are 32-bit ints, and TYPE must be 1, 2, or 3.
 * If an error occurs, a message is written to std::clog and false is returned.
 */
bool parse_object(const std::string line, Object& object, std::string& error) {

    const auto fields = split_string(line, ';');
    if (fields.size() != 4) {
        error = "incorrect number of semicolons";
        return false;
    }

    std::string field = "ID=";
    if (fields[0].rfind(field, 0) != 0) {
        error = "the first field did not start with " + field;
        return false;
    }
    auto id_string = fields[0].substr(field.length());

    field = "X=";
    if (fields[1].rfind(field, 0) != 0) {
        error = "the first field did not start with " + field;
        return false;
    }
    auto x_string = fields[1].substr(field.length());

    field = "Y=";
    if (fields[2].rfind(field, 0) != 0) {
        error = "the first field did not start with " + field;
        return false;
    }
    auto y_string = fields[2].substr(field.length());

    field = "TYPE=";
    if (fields[3].rfind(field, 0) != 0) {
        error = "the first field did not start with " + field;
        return false;
    }
    auto type_string = fields[3].substr(field.length());

    try {
        object.id   = std::stoll( id_string);
        object.x    = std::stol(   x_string);
        object.y    = std::stol(   y_string);
        object.type = std::stol(type_string);
    } catch (std::exception const& e) {
        error = "invalid field value";
        return false;
    }

    if (object.type != 1 && object.type != 2 && object.type != 3) {
        error = "invalid type";
        return false;
    }

    error = "success";
    return true;
}

/*
 * Assign a color to the object based on its position, type, and category.
 */
void color_object(Object& object) {
    /*
     * Color objects according to these conditions:
     * Category 2 objects: Yellow unless closer than 100 from the designated coordinate then red.
     * Type 1 objects: Green unless closer than 75 from the designated coordinate then yellow and if closer than 50 then red.
     * Type 2 objects: Green unless closer than 50 from the designated coordinate then yellow.
     */

    const float dist = hypot(object.x - DESIGNATED_X, object.y - DESIGNATED_Y);

    if (object.type == 3) {
        // Category 2
        object.color = YELLOW;
        if (dist < 100) {
            object.color = RED;
        }
    } else {
        // Category 1
        object.color = GREEN;
        if (object.type == 1) {
            if (dist < 50) {
                object.color = RED;
            } else if (dist < 75) {
                object.color = YELLOW;
            }
        } else if (object.type == 2 && dist < 50) {
            object.color = YELLOW;
        }
    }
}

/*
 * Add the object to the global list of objects. If an object with
 * the same ID already exists, it is replaced by the new object.
 */
void add_or_update_object(Object object) {
    objects_mutex.lock();

    auto found = false;
    for (std::size_t i = 0; i < objects.size(); i++) {
        if (object.id == objects[i].id) {
            objects[i] = object; // Update object
            found = true;
            break;
        }
    }

    if (!found)  objects.push_back(object); // Add object

    objects_mutex.unlock();
}

/*
 * Print info on the global list of objects. A preamble and the
 * number of objects are printed first, followed by the members
 * of each object. All values are printed as zero-padded hex
 * values. The padding is necessary because otherwise it would
 * be impossible to separate the values.
 */
void relay_info_once(std::ostream &os) {
    objects_mutex.lock();

    os << std::hex; // Print hexadecimals
    auto old_filler = os.fill(); // Store the old fill char
    os.fill('0'); // Zero-padding

    // Print the preamble
    const int32_t preamble = 0xfeff;
    os << std::setw(sizeof(preamble)*2) << preamble;

    const int32_t count = objects.size();
    os << std::setw(sizeof(count)*2) << count;

    for (auto object : objects) {
        os << std::setw(sizeof(object.   id)*2) << object.   id;
        os << std::setw(sizeof(object.    x)*2) << object.    x;
        os << std::setw(sizeof(object.    y)*2) << object.    y;
        os << std::setw(sizeof(object. type)*2) << object. type;
        os << std::setw(sizeof(object.color)*2) << object.color;
    }

    os.fill(old_filler); // Restore the old fill char
    os << std::dec; // Stop printing hexadecimals

    objects_mutex.unlock();
}

bool do_relay = true; // Informs the relay thread when to terminate

/*
 * Relay info about the objects in the global list with fixed time intervals.
 */
void relay_info_continually() {
    while (do_relay) {
        relay_info_once(std::cout);
        std::cout << std::endl;
        std::this_thread::sleep_for(RELAY_INTERVAL);
    }
}

/*
 * Start the socket communication with the server. Upon connecting, this
 * function accepts data from the server and parses it as it comes. A
 * child thread is spawned that continually relays info gathered from
 * said data. This function blocks the thread it's called from until the
 * server disconnects or an error occurs. If an error occurs, the error
 * is printed to std::clog and a non-zero value is returned.
 */
int start_client(const char *server_ip, const char *server_port) {

    // Init Winsock
    WORD wVersionRequested = MAKEWORD(2,2);
    WSADATA lpWSAData;
    int error_code = WSAStartup(wVersionRequested, &lpWSAData);
    if (error_code) {
        std::clog << "WSAStartup() failed with the error code " << error_code << std::endl;
        return 1;
    }

    // Set hints for the socket communication
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;   // Unspecified address family
    hints.ai_socktype = SOCK_STREAM; // Byte stream socket
    hints.ai_protocol = IPPROTO_TCP; // Use TCP

    // Resolve the server address and port
    struct addrinfo *addrinfos = nullptr; // Linked list
    error_code = getaddrinfo(server_ip, server_port, &hints, &addrinfos);
    if (error_code) {
        std::clog << "getaddrinfo() failed with the error code " << error_code << std::endl;
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    SOCKET sock = INVALID_SOCKET;
    struct addrinfo *ptr = nullptr;
    for (ptr = addrinfos; ptr != nullptr; ptr = ptr->ai_next) {

        // Create the socket
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock == INVALID_SOCKET) {
            std::clog << "socket() failed with the error code " << WSAGetLastError() << std::endl;
            WSACleanup();
            return 1;
        }

        // Connect to the server
        error_code = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (error_code) {
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }

        break; // Success!
    }

    freeaddrinfo(addrinfos);

    if (sock == INVALID_SOCKET) {
        std::clog << "Failed to connect" << std::endl;
        WSACleanup();
        return 1;
    }

    // Start receiving data on a separate thread
    std::thread relay_thread(relay_info_continually);

    // Receive data until we stop receiving, i.e., the connection closes
    char receive_buffer[RECEIVE_BUFFER_LENGTH];
    int bytes_received;
    do {
        // Note: recv() clears the buffer before adding data
        bytes_received = recv(sock, receive_buffer, RECEIVE_BUFFER_LENGTH, 0);

        auto lines = split_string(receive_buffer, '\n');

        for (auto line : lines) {
            // Skip empty lines. Maybe we should check for blank lines as well?
            if (!line.length())  continue;

            Object object;
            std::string error;
            bool ok = parse_object(line, object, error);
            if (ok) {
                color_object(object);
                add_or_update_object(object);
            } else {
                std::clog << "Could not parse the line below (" << error << ")" << std::endl;
                std::clog << line << std::endl;
            }
        }

        /*
        // Debug print
        if (bytes_received > 0) {
            std::cout << "Bytes received: " << bytes_received << std::endl;
        } else if (bytes_received == 0) {
            std::cout << "Connection closed" << std::endl;
        } else {
            std::cout << "recv() failed with error: " << WSAGetLastError() << std::endl;
        }
        */

    } while (bytes_received > 0);

    // TODO: Catch keyboard interrupts to exit gracefully

    do_relay = false; // Make the relay thread quit
    relay_thread.join();
    closesocket(sock);
    WSACleanup();

    return 0;
}
