#include <stdio.h>
#include <stdlib.h>

typedef struct Strizzle {
    char* items;
    size_t capacity;
    size_t length;
} Strizzle;

int read_entire_file(char* path, Strizzle* sb) {
    FILE* fp;
    fp = fopen(path, "rb");
    if (fp == NULL) {
        printf("Failed to open the path %s\n", path);
        return 0;
    }

    if (fseek(fp, 0, SEEK_END) < 0) {
        printf("Failed to move the pointer to the last\n");
        return 0;
    }
    
    long long size = ftell(fp);
    rewind(fp);

    sb -> items = (char*)malloc(size + 1);
    fread(sb -> items, size, 1, fp);
    
    sb -> length = size;
    sb -> capacity = size;
    
    fclose(fp);
    return 1;
}
