#CXXFLAGS=-DNDEBUG -O3
CXXFLAGS=-O2 -g

all: channel_extractor

OBJ = main.o utils.o benchmark.o channel_extractor_brute_force.o
channel_extractor: $(OBJ)
	g++ $(CXXFLAGS) -o channel_extractor $(OBJ)

clean:
	rm -f channel_extractor $(OBJ)