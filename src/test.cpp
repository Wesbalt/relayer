#include "client.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <limits>

int assert_count = 0;
int failed_assert_count = 0;

void assert(bool condition, std::string message) {
    assert_count++;
    if (!condition) {
        failed_assert_count++;
        std::cerr << "\tFail: " << message << std::endl;
    }
}

void test_split_string() {
    std::cout << "split_string()" << std::endl;

    std::cout << "\tTest case: empty string" << std::endl;
    auto str = "";
    auto substrings = split_string(str, ' ');
    if (substrings.size() == 1) {
        assert(substrings[0] == "", "bad first substring");
    } else {
        assert(false, "incorrect number of substrings");
    }

    std::cout << "\tTest case: separator does not exist" << std::endl;
    str = "XXXXX";
    substrings = split_string(str, 'x');
    if (substrings.size() == 1) {
        assert(substrings[0] == "XXXXX", "bad first substring");
    } else {
        assert(false, "incorrect number of substrings");
    }

    std::cout << "\tTest case: words" << std::endl;
    str = "These are words";
    substrings = split_string(str, ' ');
    if (substrings.size() == 3) {
        assert(substrings[0] == "These", "bad first substring");
        assert(substrings[1] ==   "are", "bad second substring");
        assert(substrings[2] == "words", "bad third substring");
    } else {
        assert(false, "incorrect number of substrings");
    }

    std::cout << "\tTest case: hyphens" << std::endl;
    str = "I-sure-love-hyphens";
    substrings = split_string(str, '-');
    if (substrings.size() == 4) {
        assert(substrings[0] ==       "I", "bad first substring");
        assert(substrings[1] ==    "sure", "bad second substring");
        assert(substrings[2] ==    "love", "bad third substring");
        assert(substrings[3] == "hyphens", "bad fourth substring");
    } else {
        assert(false, "incorrect number of substrings");
    }

    std::cout << "\tTest case: consecutive splits" << std::endl;
    str = "head xxx tail";
    substrings = split_string(str, 'x');
    if (substrings.size() == 4) {
        assert(substrings[0] ==  "head ", "bad first substring");
        assert(substrings[1] ==       "", "bad second substring");
        assert(substrings[2] ==       "", "bad third substring");
        assert(substrings[3] ==  " tail", "bad fourth substring");
    } else {
        assert(false, "incorrect number of substrings");
    }

    std::cout << "\tTest case: split at start" << std::endl;
    str = "split me immediately please!";
    substrings = split_string(str, 's');
    if (substrings.size() == 3) {
        assert(substrings[0] ==                         "", "bad first substring");
        assert(substrings[1] == "plit me immediately plea", "bad second substring");
        assert(substrings[2] ==                       "e!", "bad second substring");
    } else {
        assert(false, "incorrect number of substrings");
    }

    std::cout << "\tTest case: split at end" << std::endl;
    str = "Now, do it at the end";
    substrings = split_string(str, 'd');
    if (substrings.size() == 3) {
        assert(substrings[0] ==          "Now, ", "bad first substring");
        assert(substrings[1] == "o it at the en", "bad second substring");
        assert(substrings[2] ==               "", "bad second substring");
    } else {
        assert(false, "incorrect number of substrings");
    }
}

void test_parse_object() {
    std::cout << "parse_object()" << std::endl;

    std::string line;
    std::stringstream ss;
    std::string placeholder; // Just something to pass as the error message in the function
    Object object;
    bool ok;

    std::cout << "\tTest case: a valid line" << std::endl;
    line = "ID=123;X=1;Y=2;TYPE=3";
    ok = parse_object(line, object, placeholder);
    assert(ok, "failed to parse valid line");
    assert(object.id   == 123, "bad ID");
    assert(object.x    ==   1, "bad X");
    assert(object.y    ==   2, "bad Y");
    assert(object.type ==   3, "bad type");

    std::cout << "\tTest case: another valid line" << std::endl;
    line = "ID=2691882127234543;X=199;Y=230;TYPE=1";
    ok = parse_object(line, object, placeholder);
    assert(ok, "failed to parse valid line");
    assert(object.id   == 2691882127234543, "bad ID");
    assert(object.x    == 199, "bad X");
    assert(object.y    == 230, "bad Y");
    assert(object.type ==   1, "bad type");

    std::cout << "\tTest case: smallest possible ints" << std::endl;
    ss.str(""); // Flush
    ss << "ID=" << std::numeric_limits<int64_t>::min();
    ss << ";X=" << std::numeric_limits<int32_t>::min();
    ss << ";Y=" << std::numeric_limits<int32_t>::min();
    ss << ";TYPE=1";
    line = ss.str();
    ok = parse_object(line, object, placeholder);
    assert(ok, "failed to parse valid line");
    assert(object.id == std::numeric_limits<int64_t>::min(), "bad ID");
    assert(object.x  == std::numeric_limits<int32_t>::min(), "bad X");
    assert(object.y  == std::numeric_limits<int32_t>::min(), "bad Y");

    std::cout << "\tTest case: largest possible ints" << std::endl;
    ss.str(""); // Flush
    ss << "ID=" << std::numeric_limits<int64_t>::max();
    ss << ";X=" << std::numeric_limits<int32_t>::max();
    ss << ";Y=" << std::numeric_limits<int32_t>::max();
    ss << ";TYPE=1";
    line = ss.str();
    ok = parse_object(line, object, placeholder);
    assert(ok, "failed to parse valid line");
    assert(object.id == std::numeric_limits<int64_t>::max(), "bad ID");
    assert(object.x  == std::numeric_limits<int32_t>::max(), "bad X");
    assert(object.y  == std::numeric_limits<int32_t>::max(), "bad Y");

    std::cout << "\tTest case: 1 smaller than smallest possible ints" << std::endl;
    // This string is hard-coded for obvious reasons...
    line = "ID=-9223372036854775809;X=-2147483649;Y=-2147483649;TYPE=1";
    ok = parse_object(line, object, placeholder);
    assert(!ok, "successfully parsed invalid line");

    std::cout << "\tTest case: 1 larger than largest possible ints" << std::endl;
    // This string is hard-coded for obvious reasons...
    line = "ID=9223372036854775808;X=2147483648;Y=2147483648;TYPE=1";
    ok = parse_object(line, object, placeholder);
    assert(!ok, "successfully parsed invalid line");

    std::cout << "\tTest case: empty line" << std::endl;
    line = "";
    ok = parse_object(line, object, placeholder);
    assert(!ok, "successfully parsed invalid line");

    std::cout << "\tTest case: whitespace line" << std::endl;
    line = "\t \r\n \n\t \r    ";
    ok = parse_object(line, object, placeholder);
    assert(!ok, "successfully parsed invalid line");

    std::cout << "\tTest case: missing X" << std::endl;
    line = "ID=2691882127221587;Y=310;TYPE=2";
    ok = parse_object(line, object, placeholder);
    assert(!ok, "successfully parsed invalid line");

    std::cout << "\tTest case: invalid Y" << std::endl;
    line = "ID=123;X=1;Y=xyz;TYPE=3";
    ok = parse_object(line, object, placeholder);
    assert(!ok, "successfully parsed invalid line");

    std::cout << "\tTest case: extra field" << std::endl;
    line = "ID=123;X=1;Y=2;OTHER=10;TYPE=3";
    ok = parse_object(line, object, placeholder);
    assert(!ok, "successfully parsed invalid line");

    std::cout << "\tTest case: extra separator" << std::endl;
    line = "ID=123;X=1;;Y=2;TYPE=3";
    ok = parse_object(line, object, placeholder);
    assert(!ok, "successfully parsed invalid line");

    std::cout << "\tTest case: newline in line" << std::endl;
    line = "ID=123;X=1;\nY=2;TYPE=3";
    ok = parse_object(line, object, placeholder);
    assert(!ok, "successfully parsed invalid line");

    std::cout << "\tTest case: TYPE too small" << std::endl;
    line = "ID=123;X=1;Y=2;TYPE=0";
    ok = parse_object(line, object, placeholder);
    assert(!ok, "successfully parsed invalid line");

    std::cout << "\tTest case: TYPE too large" << std::endl;
    line = "ID=123;X=1;Y=2;TYPE=4";
    ok = parse_object(line, object, placeholder);
    assert(!ok, "successfully parsed invalid line");
}

void test_color_object() {
    std::cout << "color_object()" << std::endl;

    Object object;
    object.id = 0;

    std::cout << "\tTest case: close category 2" << std::endl;
    object.x    = DESIGNATED_X - 20;
    object.y    = DESIGNATED_Y + 20;
    object.type = 3;
    color_object(object);
    assert(object.color == RED, "bad color");

    std::cout << "\tTest case: far away category 2" << std::endl;
    // Distance ~ 282.8
    object.x    = DESIGNATED_X - 200;
    object.y    = DESIGNATED_Y + 200;
    object.type = 3;
    color_object(object);
    assert(object.color == YELLOW, "bad color");

    std::cout << "\tTest case: category 2 just inside boundary" << std::endl;
    // Distance ~ 99.98
    object.x    = DESIGNATED_X + 99;
    object.y    = DESIGNATED_Y + 14;
    object.type = 3;
    color_object(object);
    assert(object.color == RED, "bad color");

    std::cout << "\tTest case: category 2 just outside boundary" << std::endl;
    // Distance ~ 100.005
    object.x    = DESIGNATED_X + 100;
    object.y    = DESIGNATED_Y + 1;
    object.type = 3;
    color_object(object);
    assert(object.color == YELLOW, "bad color");

    std::cout << "\tTest case: close type 1" << std::endl;
    // Distance ~ 36.1
    object.x    = DESIGNATED_X - 20;
    object.y    = DESIGNATED_Y + 30;
    object.type = 1;
    color_object(object);
    assert(object.color == RED, "bad color");

    std::cout << "\tTest case: far away type 1" << std::endl;
    // Distance ~ 360.6
    object.x    = DESIGNATED_X - 200;
    object.y    = DESIGNATED_Y + 300;
    object.type = 1;
    color_object(object);
    assert(object.color == GREEN, "bad color");

    std::cout << "\tTest case: type 1 just inside first boundary" << std::endl;
    // Distance ~ 49.2
    object.x    = DESIGNATED_X - 20;
    object.y    = DESIGNATED_Y + 45;
    object.type = 1;
    color_object(object);
    assert(object.color == RED, "bad color");

    std::cout << "\tTest case: type 1 just outside first boundary" << std::endl;
    // Distance ~ 50.2
    object.x    = DESIGNATED_X - 20;
    object.y    = DESIGNATED_Y + 46;
    object.type = 1;
    color_object(object);
    assert(object.color == YELLOW, "bad color");

    std::cout << "\tTest case: type 1 just inside second boundary" << std::endl;
    // Distance ~ 74.6
    object.x    = DESIGNATED_X - 63;
    object.y    = DESIGNATED_Y + 40;
    object.type = 1;
    color_object(object);
    assert(object.color == YELLOW, "bad color");

    std::cout << "\tTest case: type 1 just outside second boundary" << std::endl;
    // Distance ~ 75.5
    object.x    = DESIGNATED_X - 64;
    object.y    = DESIGNATED_Y + 40;
    object.type = 1;
    color_object(object);
    assert(object.color == GREEN, "bad color");

    std::cout << "\tTest case: close type 2" << std::endl;
    // Distance ~ 40.3
    object.x    = DESIGNATED_X + 5;
    object.y    = DESIGNATED_Y + 40;
    object.type = 2;
    color_object(object);
    assert(object.color == YELLOW, "bad color");

    std::cout << "\tTest case: far away type 2" << std::endl;
    // Distance ~ 501.6
    object.x    = DESIGNATED_X + 500;
    object.y    = DESIGNATED_Y - 40;
    object.type = 2;
    color_object(object);
    assert(object.color == GREEN, "bad color");

    std::cout << "\tTest case: type 2 just inside boundary" << std::endl;
    // Distance ~ 49.03
    object.x    = DESIGNATED_X - 10;
    object.y    = DESIGNATED_Y - 48;
    object.type = 2;
    color_object(object);
    assert(object.color == YELLOW, "bad color");

    std::cout << "\tTest case: type 2 just outside boundary" << std::endl;
    // Distance ~ 50.3
    object.x    = DESIGNATED_X - 15;
    object.y    = DESIGNATED_Y - 48;
    object.type = 2;
    color_object(object);
    assert(object.color == GREEN, "bad color");
}

void test_add_or_update_object() {
    std::cout << "add_or_update_object()" << std::endl;

    // Define objects used for testing purposes
    Object o1, o2, o3, o4, o5;

    o1.id   = 123;
    o1.x    = 30;
    o1.y    = 50;
    o1.type = 1;

    o2.id   = 456;
    o2.x    = 30;
    o2.y    = 50;
    o2.type = 2;

    o3.id   = 789;
    o3.x    = 30;
    o3.y    = 50;
    o3.type = 3;

    o4      = o1;
    o4.x    = 1000;
    o4.type = 3;

    o5      = o2;
    o4.y    = -1000;
    o4.type = 1;

    std::cout << "\tTest case: adding unique objects" << std::endl;
    objects.clear();
    add_or_update_object(o1);
    add_or_update_object(o2);
    add_or_update_object(o3);
    if (objects.size() == 3) {
        assert(objects[0] == o1, "wrong first object");
        assert(objects[1] == o2, "wrong second object");
        assert(objects[2] == o3, "wrong third object");
    } else {
        assert(false, "unexpected list length");
    }

    std::cout << "\tTest case: updating same objects" << std::endl;
    objects.clear();
    add_or_update_object(o1);
    add_or_update_object(o2);
    add_or_update_object(o3);
    add_or_update_object(o4); // Update o1
    add_or_update_object(o5); // Update o2
    if (objects.size() == 3) {
        assert(objects[0] == o4, "wrong first object");
        assert(objects[1] == o5, "wrong second object");
        assert(objects[2] == o3, "wrong third object");
    } else {
        assert(false, "unexpected list length");
    }

    objects.clear(); // Remove side effects
}

void test_relay_info_once() {
    std::cout << "relay_info_once()" << std::endl;

    std::stringstream ss;
    std::string preamble = "0000feff"; // TODO: Don't hard code this?
    std::string expected_string;

    // Define objects used for testing purposes. Their fields
    // are written as zero-padded hex literals only for the
    // sake of easily cross-referencing them with the
    // assertions in the following test cases.
    Object o1, o2, o3;

    o1.id    = 0x00000000000000ab;
    o1.x     = 0x000000cd;
    o1.y     = 0x000000ef;
    o1.type  = 0x00000001;
    o1.color = 0x00002345;

    o2.id    = 0x83d74892fc8a1997;
    o2.x     = 0xdeadbeef;
    o2.y     = 0x1e022c34;
    o2.type  = 0x8ed5452b;
    o2.color = 0xb901dc4b;

    o3.id    = 0x96d14c9f09f411d4;
    o3.x     = 0x51d0cdb6;
    o3.y     = 0xc183213f;
    o3.type  = 0xd9def867;
    o3.color = 0xda61c64d;

    std::cout << "\tTest case: relay zero objects" << std::endl;
    objects.clear();
    ss.str(""); // Flush
    relay_info_once(ss);
    expected_string = preamble + "00000000";
    assert(ss.str() == expected_string, "bad output");

    std::cout << "\tTest case: relay one object" << std::endl;
    objects.clear();
    ss.str(""); // Flush
    add_or_update_object(o1);
    relay_info_once(ss);
    expected_string = preamble;
    expected_string += "00000001";
    // Append output from o1
    expected_string += "00000000000000ab";
    expected_string += "000000cd";
    expected_string += "000000ef";
    expected_string += "00000001";
    expected_string += "00002345";
    assert(ss.str() == expected_string, "bad output");

    std::cout << "\tTest case: relay many objects" << std::endl;
    objects.clear();
    ss.str(""); // Flush
    add_or_update_object(o1);
    add_or_update_object(o2);
    add_or_update_object(o3);
    relay_info_once(ss);
    expected_string = preamble;
    expected_string += "00000003";
    // Append output from o1
    expected_string += "00000000000000ab";
    expected_string += "000000cd";
    expected_string += "000000ef";
    expected_string += "00000001";
    expected_string += "00002345";
    // Append output from o2
    expected_string += "83d74892fc8a1997";
    expected_string += "deadbeef";
    expected_string += "1e022c34";
    expected_string += "8ed5452b";
    expected_string += "b901dc4b";
    // Append output from o3
    expected_string += "96d14c9f09f411d4";
    expected_string += "51d0cdb6";
    expected_string += "c183213f";
    expected_string += "d9def867";
    expected_string += "da61c64d";
    assert(ss.str() == expected_string, "bad output");

    objects.clear(); // Remove side effects
    ss.str(""); // Flush
}

int main() {
    std::cout << std::endl << "Running test suite..." << std::endl;

    test_split_string();
    test_parse_object();
    test_color_object();
    test_add_or_update_object();
    test_relay_info_once();

    std::cout << "Tests complete (" << failed_assert_count << "/" << assert_count << " asserts failed)" << std::endl;
}
