TARGET := ps3eye_sdl

SOURCES := $(wildcard *.cpp ../src/*.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))

CXXFLAGS += -I../src -I. -std=c++11 -O3

ifeq ($(OS),Windows_NT)
    TARGET := ps3eye_sdl.exe
    CXXFLAGS += -Iinclude -D_THREAD_SAFE
    LIBS += -Llib/MinGW64 -lmingw32 -lSDL2main -lSDL2 -lusb-1.0
    
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

clean:
	rm -f $(TARGET) $(OBJECTS)
    
install:
	mv $(TARGET) ./bin