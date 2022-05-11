OUT	= sniffer
CC	 = g++
FLAGS = -O3 -g -std=c++17 -o
OFLAGS = -c -O3 -g -Wall -std=c++17 -o
OBJECTS = utils.o manager.o worker.o 
SOURCE = utils.cpp manager.cpp worker.cpp 

all: $(OBJECTS) $(SOURCE)
	$(CC) $(OFLAGS) utils.o utils.cpp
	$(CC) $(OFLAGS) worker.o worker.cpp
	$(CC) $(OFLAGS) manager.o manager.cpp
	$(CC) $(FLAGS) $(OUT) $(OBJECTS) main.cpp

clean: 
	rm -f $(OBJECTS)
