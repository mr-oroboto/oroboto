CC=g++
RM=/bin/rm
CFLAGS=-c -Wall -I../libs -I../modules
LDFLAGS=-pthread
SOURCES=main.cpp ../libs/motorlib.cpp ../libs/sysfslib.cpp ../libs/pwmlib.cpp ../libs/adclib.cpp ../libs/gpio.cpp ../libs/led.cpp ../libs/odo.cpp ../libs/dotlog.cpp ../libs/logger.cpp ../modules/controller.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=demo_gotogoal

all: $(SOURCES) $(EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	$(RM) *.o ../libs/*.o ../modules/*.o $(EXECUTABLE)
		