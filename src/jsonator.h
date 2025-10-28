#include <complex.h>

#ifndef JSONATOR_H
#define JSONATOR_H
typedef enum {
	JR_STRING = 1,
	JR_INT_NUMBER,
	JR_DECIMAL_NUMBER,
	JR_OBJECT,
	JR_ARRAY,
	JR_BOOLEAN,
	JR_NULL,
} JSONATOR_TYPE;

typedef struct JSONATOR {
	char* key;
	JSONATOR_TYPE type;

	char* str;
	long long number;
	double float_num;
	short int bool_value;

	struct JSONATOR* object;
	struct JSONATOR* next;

} JSONATOR;

typedef struct JSON_RESULT {
	short int status;
	char* message;
} JSON_RESULT;

int parse_json(const char* str);
#endif
