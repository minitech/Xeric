typedef struct {
	int capacity;
	int length;
	char * pointer;
} string_buffer;

string_buffer sb_alloc() {
	string_buffer buf;

	buf.capacity = 100;
	buf.length = 0;
	buf.pointer = malloc(100);

	return buf;
}

void sb_free(string_buffer buf) {
	free(buf.pointer);
}

void sb_append(string_buffer * buf, char c) {
	if(buf->length + 1 >= buf->capacity) {
		buf->capacity += buf->capacity / 10;
		buf->pointer = realloc(buf->pointer, buf->capacity);
	}

	buf->pointer[buf->length++] = c;
}

char * sb_tostring(string_buffer buf) {
	char * result = malloc(buf.length + 1);
	int i;

	for(i = 0; i < buf.length; i++) {
		result[i] = buf.pointer[i];
	}

	result[i] = '\0';

	return result;
}