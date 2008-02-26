CXXFLAGS=-DNDEBUG -O3

all: channel_extractor

OBJ = main.o utils.o 
channel_extractor: $(OBJ)
	g++ $(CXXFLAGS) -o channel_extractor $(OBJ)
