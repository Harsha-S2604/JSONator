typedef struct StringBuilder {
	const char* data;
	size_t capacity;
	size_t length;
	size_t offset;
} StringBuilder;

StringBuilder new_string_builder() {
	StringBuilder sb = { 0 };
	return sb;	
}

