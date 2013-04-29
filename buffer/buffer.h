

#define OUTPUT_BUFFER_SIZE 512 // should be based on size in tty / preferably less than page size.
// less elements can be stored in the buffer than the size. (e.g. 1 / 2 less)

#if OUTPUT_BUFFER_SIZE <= 2
	#error OUTPUT_BUFFER_SIZE must be bigger
#endif

typedef char bufferEntry;

typedef struct buffer *Buffer;

struct buffer { //TODO: rename
	int totalLengthThatShouldNotBeChangedAndIsNotZero;
	volatile int readPosition;
	volatile int writePosition;
	bufferEntry data[OUTPUT_BUFFER_SIZE];
};





struct buffer createBuffer(void);

void addToBuffer(Buffer buffer, bufferEntry valueToAdd);
char getAndRemoveFromBuffer(Buffer buffer); // returns VALUE_ON_READ_FAILING on failure


#define VALUE_ON_READ_FAILING '\0'