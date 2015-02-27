TARGET := ps3eye_sdl

SOURCES := $(wildcard *.cpp ../src/*.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))

CXXFLAGS += -I../src -I. -std=c++11 -O3

# You need to have libusb-1.0, e.g. installed via Homebrew
CXXFLAGS += $(shell pkg-config --cflags libusb-1.0)
LIBS += $(shell pkg-config --libs libusb-1.0)

# Also SDL 1.3 (SDL 2.0 will need customizations), e.g. via Homebrew
CXXFLAGS += $(shell sdl-config --cflags)
LIBS += $(shell sdl-config --libs)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS)

debug: CXXFLAGS += -DDEBUG -g
debug: CCFLAGS += -DDEBUG -g
debug: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS)
