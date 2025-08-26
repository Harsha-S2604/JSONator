#include <stdio.h>
#include "strizzle.h"

int parse_file(char* path) {
    Strizzle sb = {0};
    int file_parsed = read_entire_file(path, &sb);
    if (!file_parsed) {
        printf("Failed to parse the file\n");
        return 0;
    }

    fwrite(sb.items, sb.length, 1, stdout);
    return 1;
}

void parse_json(char* path) {

}

int main() {
    char* path = "profile.json";
    int file_parsed = parse_file(path);
    return 0;
}
