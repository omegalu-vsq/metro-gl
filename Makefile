CC = g++

CPP_FILES = camera.cpp matrix.cpp program.cpp vector.cpp object.cpp scene.cpp metrogl.cpp
HXX_FILES = camera.hh matrix.hh program.hh vector.hh object.hh scene.hh metrogl.hh
OBJ_FILES = $(CPP_FILES:.cpp=.o)

CXX_FLAGS += -Wall -Wextra -O3 -g -std=c++14
CXX_FLAGS += -m64 -march=native
CXX_FLAGS += -fopt-info-vec-optimized
LDXX_FLAGS = -lGL -lGLEW -lglut -lpthread -lassimp

MAIN_FILE = main.cpp
DIST = metro-gl

.PHONY: all clean

all: $(DIST)

$(DIST): $(MAIN_FILE) $(OBJ_FILES)
	$(CC) $(MAIN_FILE) -o $(DIST) $(OBJ_FILES) $(CXX_FLAGS) $(LDXX_FLAGS)

%.o: %.cpp $(HXX_FILES)
	$(CC) -c -o $@ $< $(CXX_FLAGS)

clean:
	rm -f $(OBJ_FILES) $(DIST)
