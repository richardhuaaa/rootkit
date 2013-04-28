typedef struct buffer *Buffer;

typedef char bufferEntry;

Buffer createBuffer(int size);
void destroyBuffer(Buffer buffer);


void addToBuffer(Buffer buffer, bufferEntry valueToAdd);
char getAndRemoveFromBuffer(Buffer buffer); // returns VALUE_ON_READ_FAILING on failure


#define VALUE_ON_READ_FAILING '\0'