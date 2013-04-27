# Target rules
outfileFile = testBuffer

default: compile run

compile:
	gcc -Wall -Werror -O -o $(outfileFile) testBuffer.c buffer.c
run:
	./$(outfileFile)