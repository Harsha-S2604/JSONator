#include <stdio.h>
#include "jsonator.h"

int main() {
    printf("JSONATOR\n");
	char* input = "profile.json";
	int parsed = parse_json(input);
	return 0;
}
