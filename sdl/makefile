TARGET := ps3eye_sdl

SOURCES := $(wildcard *.cpp ../src/*.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))

CXXFLAGS += -I../src -I. -std=c++11 -O3

# You need to have libusb-1.0, e.g. installed via MacPorts or Homebrew
CXXFLAGS += $(shell pkg-config --cflags libusb-1.0)
LIBS += $(shell pkg-config --libs libusb-1.0)

# Also SDL 2.0, e.g. via MacPorts or Homebrew
CXXFLAGS += $(shell sdl2-config --cflags)
LIBS += $(shell sdl2-config --libs)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET) $(OBJECTS)
