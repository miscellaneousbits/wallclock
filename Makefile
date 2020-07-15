CC=gcc

INSTALLDIR = /usr/local/bin
BLUEZDIR = ../bluez-5.50

INCLUDES = -I$(BLUEZDIR)

LIBS = -lm -lbluetooth -lsystemd -lpthread -lbcm2835

COPTFLAG = -O3
#COPTFLAG = -g

CFLAGS = $(COPTFLAG) -Wall -ffunction-sections -fdata-sections $(INCLUDES)
LDFLAGS = $(COPTFLAG) -Wl,--gc-sections,-Map=$(TARGET).map

SOURCES = \
	$(wildcard *.c) \
	gatt-db.c \
	queue.c \
	util.c \
	mainloop.c \
	timeout-mainloop.c \
	io-mainloop.c \
	att.c \
	crypto.c \
	gatt-server.c \
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
	@echo "CCLD $(OBJECTS)"
	$(ECHO)$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

%.o: %.c Makefile
	@echo "CC $<"
	$(ECHO)$(CC) $(CFLAGS) -MMD -c $< -o $@

clean:
	@rm -rf $(OBJECTS) $(DEPS) *.map $(TARGET)

install: $(TARGET)
	sudo cp $(TARGET) $(INSTALLDIR)
