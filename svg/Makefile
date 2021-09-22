CFLAGS=-Wall -O3 -g -Wextra -Wno-unused-parameter
CXXFLAGS=$(CFLAGS) -std=c++11
OBJECTS= watchpanel.o data.o strings.o graphics.o
BINARIES=svg2led

LDFLAGS+= -lm -lpthread -lpugixml -lfmt

all : $(BINARIES)

svg2led : svg2led.o $(OBJECTS)

% : %.o 
	$(CXX) $< -o $@ $(OBJECTS) $(LDFLAGS)

%.o : %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(BINARIES)

FORCE:
.PHONY: FORCE