#include <stdio.h>
#include <string.h>
#include "strizzle.h"

int parse_file(const char* path, Strizzle* sb) {
    int file_parsed = read_entire_file(path, sb);
    if (!file_parsed) {
        printf("Failed to parse the file\n");
        return 0;
    }

    return 1;
}

int parse_json(const char* str, const char* type) {
    Strizzle sb = {0};
    
    if (strcmp(type, "file") == 0) {
        if (!parse_file(str, &sb)) {
            return 0;
        }
    } else {
        sb.items = (char*)malloc(sizeof(str) * strlen(str) + 1);

        strncpy(sb.items, str, strlen(str));
        sb.items[strlen(str) + 1] = '\0';
        sb.length = strlen(str);
        sb.capacity = strlen(str);
    }

    
    fwrite(sb.items, sb.length, 1, stdout);
    
    free(sb.items);
    return 1;
}

