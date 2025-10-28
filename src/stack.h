#ifndef STACK_H
#define STACK_H
typedef struct SymbolStack {
	char** stack;
	size_t top;
	size_t capacity;
} SymbolStack;

SymbolStack create_symbol_stack(size_t capacity) {
	if (!capacity) {
		fprintf(stderr, "CAPACITY is 0. default to 10\n");
		capacity = 10;
	}

	char** stack = (char**)malloc(capacity * sizeof(char*));
	if (stack == NULL) {
		fprintf(stderr, "(ERROR):: failed to allocate\n");
		SymbolStack st = { NULL, 0, 0 };
		return st;
	}
	
	SymbolStack st = { stack, 0, capacity };
	return st;
}

void push(SymbolStack* st, char* element) {
	if (st == NULL) {
		fprintf(stderr, "(ERROR):: stack must be allocated using create_symbol_stack\n");
		return;
	}

	if (st -> top >= st -> capacity) {
		size_t new_capacity = st -> capacity * 2;

		st -> stack = realloc(st -> stack, new_capacity * sizeof(char*));
		st -> capacity = new_capacity;
	}
	
	st -> stack[st -> top++] = element;	
}

char* pop(SymbolStack* st) {
	if (st == NULL) {
		fprintf(stderr, "(ERROR):: stack must be allocated using create_symbol_stack\n");
		return "";
	}

	if (st -> top == 0) {
		fprintf(stderr, "(ERROR):: Nothing to delete\n");
		return "";
	}
	
	char* poped_element = st -> stack[st -> top - 1];
	st -> top -= 1;

	return poped_element;
}
#endif
