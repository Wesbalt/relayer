#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>

// The designation all objects will be assessed against
const int DESIGNATED_X = 150;
const int DESIGNATED_Y = 150;

// Colors indicating the proximity of objects
const uint32_t RED    = 0x1B5B316D;
const uint32_t YELLOW = 0x1B5B336D;
const uint32_t GREEN  = 0x1B5B326D;

// Represents an object of interest
struct Object {
    int64_t  id;
    int32_t  x;
    int32_t  y;
    uint32_t type;
    uint32_t color;
};

inline bool operator==(const Object& lhs, const Object& rhs) {
    return (lhs.id == rhs.id) && (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.type == rhs.type);
}

inline std::ostream& operator<<(std::ostream &os, const Object &o) {
    return os << "Object{id=" << o.id << ", x=" << o.x << ", y=" << o.y << ", type=" << o.type << "}";
}

extern std::vector<Object> objects;

std::vector<std::string> split_string(const std::string str, const char sep);
bool parse_object(const std::string data, Object& object, std::string& error);
void color_object(Object& object);
void add_or_update_object(Object object);
void relay_info_once(std::ostream &os);
int start_client(const char *server_ip, const char *server_port);
