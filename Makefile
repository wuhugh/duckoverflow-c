# Makefile to build Duck Overflow 
CC=gcc
FLAGS= -Wall
LIBS= -lcurl -lz
OBJS= duckoverflow.o duckutils.o

# Note -lcurl must come after the C source file
# Not sure if this is the correct way of making the Makefile
duckoverflow: duckutils cjson duckoverflow.c
	$(CC) $(FLAGS) duckoverflow.c duckutils.o cJSON.o $(LIBS) -o do.out

duckutils: duckutils.h duckutils.c
	$(CC) $(FLAGS) -c duckutils.c

cjson: cJSON.c cJSON.h
	$(CC) $(FLAGS) -c cJSON.c

debug:
	$(CC) $(FLAGS) -g duckoverflow.c duckutils.o cJSON.o $(LIBS) -o do.out

clean:
	rm *.out *.o *.gch
