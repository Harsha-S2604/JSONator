#include <stdio.h>
#include "jsonator.h"

int main() {
    const char* path = "profile.json";
    const char* type = "data"; // "file"; // two types 1. file 2. data
    char* data = "{\"name\":\"John Doe\",\"age\":22}";
    parse_json(data, type);
    return 0;
}
