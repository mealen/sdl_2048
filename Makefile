CXX = g++
# Update these paths to match your installation
# You may also need to update the linker option rpath, which sets where to look for
# the SDL2 libraries at runtime to match your install
# SDL_LIB = -L/usr/local/lib -lSDL2 -Wl,-rpath=/usr/local/lib
#SDL_LIB = -L/usr/lib/x86_64-linux-gnu -lSDL2 -lSDL2_image -lSDL2_ttf
SDL_LIB = `sdl2-config --libs` -lSDL2_image -lSDL2_ttf
#SDL_INCLUDE = -I/usr/include

SDL_INCLUDE = `sdl2-config --cflags`
# You may need to change -std=c++11 to -std=c++0x if your compiler is a bit older
#CXXFLAGS = -Wall -c -ggdb -std=c++11 $(SDL_INCLUDE)
CXXFLAGS = -Wall -ggdb -c -std=c++98 $(SDL_INCLUDE)
LDFLAGS = $(SDL_LIB)
EXE = SDL_2048
BINARY_PATH=bin/
SOURCE_PATH=src/

all: $(BINARY_PATH)$(EXE)

$(BINARY_PATH)$(EXE): $(BINARY_PATH)TileData.o $(BINARY_PATH)board.o $(BINARY_PATH)main.o
	$(CXX) $^ $(LDFLAGS) -o $@

$(BINARY_PATH)main.o: $(SOURCE_PATH)main.cpp 
	mkdir -p $(BINARY_PATH)
	$(CXX) $(CXXFLAGS) $< -o $@
	
$(BINARY_PATH)board.o: $(SOURCE_PATH)Board.cpp $(SOURCE_PATH)Board.h
	mkdir -p $(BINARY_PATH)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BINARY_PATH)TileData.o: $(SOURCE_PATH)TileData.cpp $(SOURCE_PATH)TileData.h
	mkdir -p $(BINARY_PATH)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(BINARY_PATH)
