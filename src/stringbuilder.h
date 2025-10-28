typedef struct StringBuilder {
	char* data;
	size_t capacity;
	size_t length;
	size_t idx_pointer;
} StringBuilder;

StringBuilder new_string_builder() {
	StringBuilder sb = { 0 };
	return sb;	
}

short int allocate_memory_data(StringBuilder* sb, size_t capacity) {
	if (!sb) {
		fprintf(stderr, "(ERROR):: invalid string builder\n");
		return 0;
	}

	if (!capacity) {
		fprintf(stdout, "capacity must be > 0, defaulting to 10\n");
		capacity = 10;
	}

	sb -> data = malloc(capacity * sizeof(char));
	sb -> capacity = capacity;
	return 1;	
}

void push_str_element(StringBuilder* sb, char element) {
	if (sb -> idx_pointer >= sb -> capacity) {
		sb -> capacity = sb -> capacity * 2;
		sb -> data = realloc(sb -> data, sb -> capacity); 
	}

	sb -> data[sb -> idx_pointer++] = element;
}
