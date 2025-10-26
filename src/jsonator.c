#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "jsonator.h"
#include "stringbuilder.h"
#include "stack.h"


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
	sb -> length = size;
	sb -> capacity = size;

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

void sanitize_json_str(StringBuilder* sb) {
	// leading spaces
	while(isspace(sb -> data[sb -> idx_pointer])) {
		sb -> idx_pointer++;
	}

	// trailing spaces
	while (isspace(sb -> data[sb -> length - 1])) {
		sb -> length -= 1;
	}
}

void parse_object(StringBuilder* sb, SymbolStack* symbol_stack) {
	printf("PARSE OBJECT\n");
}

void parse_array(StringBuilder* sb, SymbolStack* symbol_stack) {
	printf("PARSE ARRAY\n");
}

void parse_string(StringBuilder* sb, SymbolStack* symbol_stack) {
	printf("PARSE STRING\n");
}

short int parse_number(StringBuilder* sb, JSONATOR* json) {
	short int is_float = 0;
	long long int number = 0;
	double float_num = 0;
	long long int decimal_place = 0;

	char token = sb -> data[sb -> idx_pointer];
	while(token != '\0' && 
		((token >= '0' && token <= '9') || (token == '.')) &&
		sb -> idx_pointer < sb -> length	
	) {
		if (token == '.' && is_float) return 0;
	
		if (token == '.') {
			is_float = 1;
			float_num = number;
		   	decimal_place = 10;
			token = sb -> data[++(sb -> idx_pointer)];
	   		continue; 
		}
		
		char num_str = sb -> data[sb -> idx_pointer];
		short int num = num_str - '0';
		if (is_float) {
			double carry = (double)num / (double)decimal_place;
			float_num = float_num + carry;
			decimal_place *= 10;
		} else {
			number = (number * 10) + num;
		}
		token = sb -> data[++(sb -> idx_pointer)];

		if (isspace(token) && sb -> idx_pointer < sb -> length) return 0;
	}
	
	if (is_float) {
		json -> type = JR_DECIMAL_NUMBER;
	   	json -> float_num = float_num; 	
	} else {
		json -> type = JR_INT_NUMBER;
		json -> number = number;
	}

	return 1;	
}

short int parse_bool(StringBuilder* sb, JSONATOR* json) {
	char* bool_str = calloc(sb -> capacity, sizeof(char));
	size_t idx = 0;

	while (sb -> idx_pointer < sb -> length) {
	   	char token = sb -> data[sb -> idx_pointer];
		if (token == '\0' || isspace(token) || token == ',') break;

		bool_str[idx++] = token;
		sb -> idx_pointer++;
	}
	
	if (sb -> idx_pointer < sb -> length && isspace(sb -> data[sb -> idx_pointer])) return 0;
	if (strcmp(bool_str, "true") != 0 && strcmp(bool_str, "false") != 0) return 0;
	
	json -> type = JR_BOOLEAN;
   	json -> bool_value = strcmp(bool_str, "true") == 0 ? 1 : 0;
	
	return 1;	
}

void parse_null(StringBuilder* sb) {
	
}

void display_json(JSONATOR* json) {
	while (json != NULL) {
		switch (json -> type) {
			case JR_INT_NUMBER:
				if (json -> key == NULL) {
					printf("%d\n", json -> number);
				} else {
					printf("%s: %d\n", json -> key, json -> number);
				}
				break;

			case JR_DECIMAL_NUMBER:
				if (json -> key == NULL) {
					printf("%f\n", json -> float_num);
				} else {
					printf("%s: %d\n", json -> key, json -> float_num);
				}
				break;

			case JR_STRING:
				if (json -> key == NULL) {
					printf("%s\n", json -> str);
				} else {
					printf("%s: %d\n", json -> key, json -> str);
				}
				break;
			
			case JR_BOOLEAN:
				const char* boolean_value = json -> bool_value ? "true": "false";
				if (json -> key == NULL) {
					printf("%s\n", boolean_value);
				} else {
					printf("%s: %s\n", json -> key, boolean_value);
				}
			default:
				break;

		}

		json = json -> object;
	}
}

JSONATOR parse(StringBuilder* sb) {
	JSONATOR json = {0}; 
	sanitize_json_str(sb);
	SymbolStack symbol_stack = create_symbol_stack(10);
   	
	short int parsed = 0;	

	while(sb -> data[sb -> idx_pointer] != '\0') {
		char token = sb -> data[sb -> idx_pointer];
		switch (token) {
			case '{':
				parse_object(sb, &symbol_stack);
				break;
			case '[':
				parse_array(sb, &symbol_stack);
				break;
			case '"':
				parse_string(sb, &symbol_stack);
				break;
			case 't':
			case 'f':
				parsed = parse_bool(sb, &json);
				if (!parsed) goto return_error; 
				break;
			case 'n':
				parse_null(sb);
				break;
			case '0' ... '9':
			case '.':
				parsed = parse_number(sb, &json);
				break;
			default:
				fprintf(stderr, "(PARSING_ERROR):: Syntax Error\n");
				break;
		}

		sb -> idx_pointer++;
	}

	display_json(&json);	
	return json;

	return_error:
		fprintf(stderr, "(PARSING ERROR):: Syntax Error\n");
		JSONATOR error_json = {0};
		return error_json;
}

int parse_json(const char* input) {
	StringBuilder sb = new_string_builder();
	const size_t input_len = strlen(input);
	JSON_RESULT json_result = extract_json_str(input, input_len, &sb);
	if (!json_result.status) {
		fprintf(stderr, "(ERROR):: FAILED TO PARSE -> %s\n", json_result.message);
		return 0;
	}

	parse(&sb);
	
	return 1;
}
