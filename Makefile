TARGET = list.exe
CC = g++ -c
LINKER = g++

all : list.o main.o list.h
	$(LINKER) list.o main.o -o $(TARGET)

list.o : list.cpp
	$(CC) list.cpp -o list.o

main.o : main.cpp
	$(CC) main.cpp -o main.o

clean :
	rm *.o