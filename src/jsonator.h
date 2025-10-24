#ifndef JSONATOR_H
#define JSONATOR_H
typedef struct JSON_RESULT {
	short int status;
	char* message;
} JSON_RESULT;

int parse_json(const char* str);
#endif
