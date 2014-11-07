PROJECT = raytracer_linux
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
CFLAGS  = -c -O2 -Wall -pedantic

all: $(PROJECT)

%.o: %.cpp
	g++ $(CFLAGS) $< -o $@

$(PROJECT): $(OBJECTS)
	g++ -s $(OBJECTS) -o $(PROJECT)

clean:
	rm $(OBJECTS) -f

