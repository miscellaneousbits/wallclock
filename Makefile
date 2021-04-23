CC=gcc

INSTALLDIR = /usr/local/bin
BLUEZDIR = bluez-5.50

INCLUDES = -I$(BLUEZDIR)

LIBS = -lm -lbluetooth -lsystemd -lpthread

COPTFLAG = -O3 -flto
#COPTFLAG = -g

CFLAGS = $(COPTFLAG) -Werror -Wfatal-errors -Wall -ffunction-sections -fdata-sections $(INCLUDES)
LDFLAGS = $(COPTFLAG) -Wl,--gc-sections,-Map=$(TARGET).map

SOURCES = \
	$(wildcard src/*.c) \
	$(wildcard $(BLUEZDIR)/src/shared/*.c) \
	uuid.c

VPATH = .:$(BLUEZDIR)/src/shared:$(BLUEZDIR)/src:$(BLUEZDIR)/lib

OBJECTS = $(SOURCES:.c=.o)
TARGET = wallclock
DEPS = $(OBJECTS:.o=.d)
ECHO = @
#ECHO =

.PHONY: clean install all

all: $(TARGET) Makefile

-include $(DEPS)

$(TARGET): $(OBJECTS) Makefile
	@echo "$(OBJECTS) -> $(TARGET)"
	$(ECHO)$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)
	$(ECHO) strip $(TARGET)

%.o: %.c Makefile
	@echo "$< -> $@"
	$(ECHO)$(CC) $(CFLAGS) -MMD -c $< -o $@

clean:
	@rm -rf $(OBJECTS) $(DEPS) *.map $(TARGET)

install: $(TARGET)
	sudo systemctl stop wallclock.service
	sudo cp $(TARGET) $(INSTALLDIR)
	sudo systemctl start wallclock.service
