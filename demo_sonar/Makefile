CC=g++
RM=/bin/rm
CFLAGS=-c -Wall -I../libs -I../modules
LDFLAGS=-pthread
SOURCES=main.cpp ../libs/motorlib.cpp ../libs/sysfslib.cpp ../libs/pwmlib.cpp ../libs/adclib.cpp ../libs/gpio.cpp ../libs/led.cpp ../libs/odo.cpp ../libs/dotlog.cpp ../libs/logger.cpp ../libs/sonar.cpp ../modules/controller.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=demo_sonar
CALIBRATE_SOURCES=calibrate.cpp ../libs/sysfslib.cpp ../libs/adclib.cpp ../libs/logger.cpp ../libs/sonar.cpp ../libs/dotlog.cpp
CALIBRATE_OBJECTS=$(CALIBRATE_SOURCES:.cpp=.o)
CALIBRATE_EXECUTABLE=calibrate

all: $(SOURCES) $(EXECUTABLE) $(CALIBRATE_EXECUTABLE)
		
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(CALIBRATE_EXECUTABLE): $(CALIBRATE_OBJECTS)
	$(CC) $(LDFLAGS) $(CALIBRATE_OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	$(RM) *.o ../libs/*.o ../modules/*.o $(EXECUTABLE)
		