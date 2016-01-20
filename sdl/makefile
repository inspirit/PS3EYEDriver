TARGET := ps3eye_sdl

SOURCES := $(wildcard *.cpp ../src/*.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))

CXXFLAGS += -I../src -I. -std=c++11 -O3

ifeq ($(OS),Windows_NT)
#MinGW-specfic settings
LIBS += -lmingw32
TARGET := ps3eye_sdl.exe

#See README.mingw64 for instructions on how to setup SDL2 dependencies
SDLROOT := ../../SDL2
CXXFLAGS += -I$(SDLROOT)/include -D_THREAD_SAFE
LIBS += -L$(SDLROOT)/x86_64-w64-mingw32/lib -lSDL2main -lSDL2

#See README.mingw64 for instructions on how to setup libusb dependencies
LIBUSBROOT := ../../libusb-1.0
CXXFLAGS += -I$(LIBUSBROOT)/include/libusb-1.0
LIBS += -L$(LIBUSBROOT)/MinGW64/static -lusb-1.0

else
# You need to have libusb-1.0, e.g. installed via MacPorts or Homebrew
CXXFLAGS += $(shell pkg-config --cflags libusb-1.0)
LIBS += $(shell pkg-config --libs libusb-1.0)

# Also SDL 2.0, e.g. via MacPorts or Homebrew
CXXFLAGS += $(shell sdl2-config --cflags)
LIBS += $(shell sdl2-config --libs)
endif

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS)

debug: CXXFLAGS += -DDEBUG -g
debug: CCFLAGS += -DDEBUG -g
debug: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS)
