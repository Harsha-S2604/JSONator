#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "jsonator.h"
#include "stringbuilder.h"


char* get_status_message(short int status_code) {
	switch (status_code) {
		case 1:
			return "SUCCESS";
		case 2:
			return "FILE_NOT_FOUND";
		case 3:
			return "INVALID_SYNTAX";
		default:
			return "UNKNOWN_ERROR";
	}
}

short int json_file_path(const char* input, const size_t length) {
	if(!input || length == 0) return 0;

	if(input[0] == '{' || input[0] == '[') return 0;

	const char* json_ext = (length > 5) ? input + (length - 5) : input;
	int is_json_ext_not_equal = strcmp(json_ext, ".json");
	if (is_json_ext_not_equal) return 0;
	return 1;
}

short int extract_json_from_file(const char* file_path, StringBuilder* sb) {
	FILE* fp = fopen(file_path, "rb");
	
	if (fp == NULL) return 2;
	
	if (fseek(fp, 0, SEEK_END)) return 0;
	
	size_t size = ftell(fp);
	rewind(fp);
	
	sb -> data = (char*)malloc(size + 1);
	fread(sb -> data, size, 1, fp);

	fclose(fp);
	return 1;
}

JSON_RESULT extract_json_str(const char* input, const size_t length, StringBuilder* sb) {
	short int is_json_file_path = json_file_path(input, length);
	JSON_RESULT json_result = { 1, "SUCCESS" };	
	if (!is_json_file_path) {
		sb -> data = input;
		sb -> length = length;
		sb -> capacity = length;
		return json_result;
	}

	short int status_code = extract_json_from_file(input, sb);
	char* status_message = get_status_message(status_code);
	if (strcmp(status_message, "SUCCESS") != 0) {
		json_result.status = 0;
		json_result.message = status_message;
	}

	return json_result;
}

int parse_json(const char* input) {
	StringBuilder sb = new_string_builder();
	const size_t input_len = strlen(input);
	JSON_RESULT json_result = extract_json_str(input, input_len, &sb);
	if (!json_result.status) {
		fprintf(stderr, "(ERROR):: FAILED TO PARSE -> %s\n", json_result.message);
		return 0;
	}
	
	return 1;
}
