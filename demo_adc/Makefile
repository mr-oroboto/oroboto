CC=g++
CFLAGS=-c -Wall -I../libs
LDFLAGS=
SOURCES=main.cpp ../libs/motorlib.cpp ../libs/sysfslib.cpp ../libs/pwmlib.cpp ../libs/adclib.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=demo_adc

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS) 
		$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
		$(CC) $(CFLAGS) $< -o $@
