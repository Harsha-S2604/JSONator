#include <stdio.h>
#include "jsonator.h"

int main() {
    printf("JSONATOR\n");
	char* json_str = "PARSING JSON";
	int parsed = parse_json(json_str);
	return 0;
}
