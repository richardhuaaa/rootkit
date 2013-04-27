typedef struct buffer *Buffer;

typedef char bufferEntry;

Buffer createBuffer(int size);
void destroyBuffer(Buffer buffer);

int getFreeSpaceInBuffer(Buffer buffer);

void addToBuffer(Buffer buffer, bufferEntry valueToAdd);


