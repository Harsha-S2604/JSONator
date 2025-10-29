#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "jsonator.h"
#include "stack.h"
#include "stringbuilder.h"

short int parse_object(StringBuilder* sb, SymbolStack* st, JSONATOR* json);
short int parse_array(StringBuilder* sb, SymbolStack* st, JSONATOR* json);
void print_array(JSONATOR* json);
void print_object(JSONATOR* json, size_t deep);

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

short int allocate_json_data(JSONATOR* json) {
	json -> object = (JSONATOR*)calloc(1, sizeof(JSONATOR));
	if (json -> object == NULL) return 0;
	return 1;
}

short int allocate_json_next(JSONATOR* json) {
	json -> next = (JSONATOR*)calloc(1, sizeof(JSONATOR));
	if (json -> next == NULL) return 0;
	return 1;
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

void sanitize_leading_spaces(StringBuilder* sb) {
	while (sb -> idx_pointer < sb -> length && isspace(sb -> data[sb -> idx_pointer])) {
		sb -> idx_pointer += 1;
	}
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

short int is_token_not_valid(char token) {
	return token == '\0' || token == ',' || isspace(token);
}

short int parse_string(
	StringBuilder* sb,
	SymbolStack* symbol_stack,
	JSONATOR* json,
	short int is_key
) {
	if (sb -> data[sb -> idx_pointer] != '"') {
		fprintf(stderr, "Expected the opening quote but got %c", sb -> data[sb -> idx_pointer]);
		return 0;	
	}
	
	push(symbol_stack, "\"");
	sb -> idx_pointer += 1;	
		
	StringBuilder str_builder = new_string_builder();
	allocate_memory_data(&str_builder, 10);
	while (sb -> idx_pointer < sb -> length) {
		char token = sb -> data[sb -> idx_pointer];
		if (token == '\\') {
			char next_token = sb -> data[++(sb -> idx_pointer)];
			switch (next_token) {
				case '"':
					push_str_element(&str_builder, '\"');
					break;	
				case '\\':
					push_str_element(&str_builder, '\\');
					break;	
				case 'n':
					push_str_element(&str_builder, '\n');
					break;	
				case 't':
					push_str_element(&str_builder, '\t');
					break;	
				case 'r':	
					push_str_element(&str_builder, '\r');
					break;
				default:
					push_str_element(&str_builder, next_token);
					break;	
			}
			
			sb -> idx_pointer += 1;
			continue;
		} else if (token == '"') {
			char* poped_element = pop(symbol_stack);
			if (strcmp(poped_element, "\"") == 0) {
				sb -> idx_pointer += 1;
				str_builder.data[str_builder.idx_pointer] = '\0';
				if (is_key) {
					json -> key = str_builder.data;
				} else {
					json -> type = JR_STRING;
					json -> str = str_builder.data;
				}

				sanitize_leading_spaces(sb);
				return 1;
			}

			return 0;
		} else if (token == '\n') {
			return 0;
		}
		push_str_element(&str_builder, token);	
		sb -> idx_pointer += 1;
	}
	
	return 0;
}

short int parse_number(StringBuilder* sb, JSONATOR* json) {
	short int is_float = 0;
	long long int number = 0;
	double float_num = 0;
	long long int decimal_place = 0;
	short int sign = 1;

	char token = sb -> data[sb -> idx_pointer];
	if (token == '-') {
		sign = -1;
		token = sb -> data[++(sb -> idx_pointer)];	
	}
	
	if(!isdigit(token)) return 0;
	while(sb -> idx_pointer < sb -> length) {
		
		if (isspace(token) || token == ',' || token == ']' || token == '}') break;
		if (token == '.' && is_float) return 0;
		if (token == '\\' || isalpha(token) || ispunct(token)) {
			if (token != '.' && token != '+' && token != '-') {
				return 0;
			}
		}

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

	}
	
	if (is_float) {
		json -> type = JR_DECIMAL_NUMBER;
	   	json -> float_num = sign * float_num; 	
	} else {
		json -> type = JR_INT_NUMBER;
		json -> number = sign * number;
	}

	sanitize_leading_spaces(sb);
	return 1;	
}

short int parse_bool(StringBuilder* sb, JSONATOR* json) {
	char* bool_str = calloc(sb -> capacity, sizeof(char));
	size_t idx = 0;

	while (sb -> idx_pointer < sb -> length) {
	   	char token = sb -> data[sb -> idx_pointer];
		if (is_token_not_valid(token)) break;

		bool_str[idx++] = token;
		sb -> idx_pointer++;
		
		if (strcmp(bool_str, "true") == 0 || strcmp(bool_str, "false") == 0) break;
	}
	
	if (strcmp(bool_str, "true") != 0 && strcmp(bool_str, "false") != 0) return 0;
	
	json -> type = JR_BOOLEAN;
   	json -> bool_value = strcmp(bool_str, "true") == 0 ? 1 : 0;
	
	sanitize_leading_spaces(sb);
	return 1;	
}

short int parse_null(StringBuilder* sb, JSONATOR* json) {
	char null_value[4];
	size_t idx = 0;
	
	while (sb -> idx_pointer < sb -> length) {
		char token = sb -> data[sb -> idx_pointer];
		if (is_token_not_valid(token)) break;

		null_value[idx++] = token;	
		sb -> idx_pointer++;
		
		if (strcmp(null_value, "null") == 0) break; 
	}
	
	if (strcmp(null_value, "null") != 0) return 0; 
	
	json -> type = JR_NULL;
	sanitize_leading_spaces(sb);
	return 1;
}

short int parse_array(StringBuilder* sb, SymbolStack* symbol_stack, JSONATOR* json) {
	if (sb -> data[sb -> idx_pointer] != '[') return 0;
	
	short int parsed, allocated = 0;
	push(symbol_stack, "[");
	sb -> idx_pointer += 1;
	sanitize_leading_spaces(sb);

	json -> type = JR_ARRAY;
	allocated = allocate_json_next(json);
	if (!allocated) return 0;
	json = json -> next;

	while (sb -> idx_pointer < sb -> length) {
		char token = sb -> data[sb -> idx_pointer];
		switch(token) {
			case '[': // array of array
				parsed = parse_array(sb, symbol_stack, json);
				if (!parsed) return 0;
				break;
			case '{': // array of objects
				parsed = parse_object(sb, symbol_stack, json);
				if (!parsed) return 0;
				break;
			case '"': // array of strings
				parsed = parse_string(sb, symbol_stack, json, 0);
				if (!parsed) return 0;
				break;
			case '0' ... '9':
			case '.':
			case '-':
				parsed = parse_number(sb, json);
				if (!parsed) return 0;
				break;
			case 't':
			case 'f':
				parsed = parse_bool(sb, json);
				if (!parsed) return 0;
				break;
			case 'n':
				parsed = parse_null(sb, json);
				if (!parsed) return 0;
				break;
			case ',':
				sb -> idx_pointer++;
				sanitize_leading_spaces(sb);
				break;
			case '\0':
				break;
			case ']':
				char* poped_element = pop(symbol_stack);
				if (strcmp(poped_element, "[") == 0) {
					sb -> idx_pointer += 1;
					sanitize_leading_spaces(sb);
					return 1;
				}
				return 0;
			default:
				fprintf(stderr, "(PARSING_ERROR):: Syntax error");
				return 0;
		}
		
		if(sb -> data[sb -> idx_pointer] != ']') {
			allocated = allocate_json_data(json);
			if (!allocated) return 0;
			json = json -> object;
		}

	}
	
	return 0;
}

short int parse_object(StringBuilder* sb, SymbolStack* symbol_stack, JSONATOR* json) {
	if (sb -> data[sb -> idx_pointer] != '{') return 0;
	
	push(symbol_stack, "{");
	sb -> idx_pointer += 1;

	json -> type = JR_OBJECT;
	short int allocated = allocate_json_next(json);
	if(!allocated) return 0;
	json = json -> next;

	short int is_key = 0, parsed = 0;
	sanitize_leading_spaces(sb);
	
	while (sb -> idx_pointer < sb -> length) {
		char token = sb -> data[sb -> idx_pointer];
		switch (token) {
			case '{':
				parsed = parse_object(sb, symbol_stack, json);
				if (!parsed) return 0;
				break;
			case '[':
				parsed = parse_array(sb, symbol_stack, json);
				if (!parsed) return 0;
				break;
			case '"':
				parsed = parse_string(sb, symbol_stack, json, !is_key);
				if (!parsed) return 0;
				if (!is_key) {
					if(sb -> data[sb -> idx_pointer] != ':') return 0;
					is_key = 1;
					sb -> idx_pointer += 1;
					sanitize_leading_spaces(sb);
					continue;
				}
				break;
			case '0' ... '9':
			case '.':
			case '-':
				parsed = parse_number(sb, json);
				if (!parsed) return 0;
				break;
			case 't':
			case 'f':
				parsed = parse_bool(sb, json);
				if (!parsed) return 0;
				break;
			case 'n':
				parsed = parse_null(sb, json);
				if (!parsed) return 0;
				break;
			default:
				if (!is_key) return 0;
				break;
		}

		if (
			!is_key &&
			sb -> data[sb -> idx_pointer] != ',' &&
			sb -> data[sb -> idx_pointer] != '}'
		) {
			return 0;
		}

		if (sb -> data[sb -> idx_pointer] == ',') {
			is_key = 0;
			sb -> idx_pointer += 1;
			sanitize_leading_spaces(sb);
			allocate_json_data(json);
			json = json -> object;
			continue;
		}

		if (sb -> data[sb -> idx_pointer] == '}') {
			char* poped_element = pop(symbol_stack);
			if (strcmp(poped_element, "{") == 0) {
				sb -> idx_pointer += 1;
				sanitize_leading_spaces(sb);
				return 1;
			}
			return 0;
		}
		
	}	
	return 0;
}

void print_tab(size_t deep) {
	while (deep > 0) {
		printf("\t");
		deep -= 1;
	}
}

void print_array(JSONATOR* json) {
	printf("[");
	while (json != NULL) {
		switch (json -> type) {
			case JR_ARRAY:
				print_array(json -> next);
				if (json -> object != NULL) printf(", ");
				break;

			case JR_STRING:
				if (json -> object == NULL) {
					printf("\"%s\"", json -> str);
				} else {
					printf("\"%s\", ", json -> str);
				}
				break;

			case JR_INT_NUMBER:
				if (json -> object == NULL) {
					printf("%lld", json -> number); 
				} else {
					printf("%lld, ", json -> number);
				}
				break;

			case JR_DECIMAL_NUMBER:
				if (json -> object == NULL) {
					printf("%lf", json -> float_num);
				} else {
					printf("%lf, ", json -> float_num);
				}
				break;

			case JR_NULL:
				if (json -> object == NULL) {
					printf("null");
				} else {
					printf("null, ");
				}
				break;

			case JR_BOOLEAN:
				char* bool_val = json -> bool_value ? "true" : "false";
				if (json -> object == NULL) {
					printf("%s", bool_val);
				} else {
					printf("%s, ", bool_val);
				}
				break;

			case JR_OBJECT:
				print_object(json -> next, 1);
				break;

			default:
				break;	
		}

		json = json -> object;
	}

	printf("]");
}

void print_object(JSONATOR* json, size_t deep) {
	printf("{\n");
	while (json != NULL) {
		print_tab(deep);
		switch (json -> type) {
			case JR_OBJECT:
				printf("%s: ", json -> key);
				print_object(json -> next, deep + 1);
				break;
			case JR_ARRAY:
				printf("%s: ", json -> key);
				print_array(json -> next);
				if (json -> object != NULL) {
					printf(",\n");
				} else {
					printf("\n");
				}
				break;
			case JR_STRING:
				if (json -> object != NULL) {
					printf("%s: %s,\n", json -> key, json -> str); 	
				} else {
					printf("%s: %s\n", json -> key, json -> str);
				}
				break;
			case JR_INT_NUMBER:
				if (json -> object != NULL) {
					printf("%s: %lld,\n", json -> key, json -> number);	
				} else {
					printf("%s: %lld\n", json -> key, json -> number);	
				}
				break;
			case JR_DECIMAL_NUMBER:
				if (json -> object != NULL) {
					printf("%s: %lf,\n", json -> key, json -> float_num);
				} else {
					printf("%s: %lf\n", json -> key, json -> float_num);
				}
				break;
			case JR_NULL:
				if (json -> object != NULL) {
					printf("%s: null,\n", json -> key);
				} else {
					printf("%s: null\n", json -> key);
				}
				break;
			case JR_BOOLEAN:
				char* boolean_value = json -> bool_value ? "true": "false";
				if (json -> object != NULL) {
					printf("%s: %s,\n", json -> key, boolean_value);
				} else {
					printf("%s: %s\n", json -> key, boolean_value);
				}
				break;
			default:
				break;	
		
		}

		json = json -> object;
	}

	print_tab(deep - 1);
	if (deep > 1) {
		printf("},\n");
	} else {
		printf("}");	
	}
}

void display_json(JSONATOR* json) {
	switch (json -> type) {
		case JR_ARRAY:
			print_array(json -> next);
			break;

		case JR_OBJECT:
			print_object(json -> next, 1);
			break;

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
			break;

		case JR_NULL:
			if (json -> key == NULL) {
				printf("null\n");
			} else {
				printf("%s: null\n", json -> key);
			}
			break;
		default:
			break;

	}

}

JSONATOR parse(StringBuilder* sb) {
	JSONATOR json = { 0 }; 
	sanitize_json_str(sb);
	SymbolStack symbol_stack = create_symbol_stack(10);
   	
	short int parsed = 0;

	while(sb -> idx_pointer < sb -> length && sb -> data[sb -> idx_pointer] != '\0') {
		char token = sb -> data[sb -> idx_pointer];
		switch (token) {
			case '{':
				parsed = parse_object(sb, &symbol_stack, &json);
				if (!parsed) goto return_error;
				break;
			case '[':
				parsed = parse_array(sb, &symbol_stack, &json);
				if (!parsed) goto return_error;
				break;
			case '"':
				parsed = parse_string(sb, &symbol_stack, &json, 0);
				if (!parsed || (sb -> idx_pointer < sb -> length)) goto return_error;
				break;
			case 't':
			case 'f':
				parsed = parse_bool(sb, &json);
				if (!parsed) goto return_error;
				if (sb -> idx_pointer < sb -> length) goto return_error;
				break;
			case 'n':
				parsed = parse_null(sb, &json);
				if (!parsed || sb -> idx_pointer < sb -> length) goto return_error; 	
				break;
			case '0' ... '9':
			case '.':
			case '-':
				parsed = parse_number(sb, &json);
				if (!parsed || sb -> idx_pointer < sb -> length) goto return_error;
				if (isspace(token) && sb -> idx_pointer < sb -> length) goto return_error;
				break;
			default:
				fprintf(stderr, "(PARSING_ERROR):: Syntax Error\n");
				break;
		}

		sb -> idx_pointer++;
	}
	
	if (symbol_stack.top > 0) goto return_error;

	display_json(&json);
	printf("\n");	
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
