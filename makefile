CC=gcc

INCLUDES= \
	-I/usr/include/dbus-1.0 \
	-I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include \
	-I/usr/include/glib-2.0 \
	-I../bluez

LIBS=-lm -lbluetooth -lsystemd -lpthread

COPTFLAG=-O3 -lfto
#COPTFLAG=-g

CFLAGS=$(COPTFLAG) -Wall -ffunction-sections -fdata-sections $(INCLUDES)
LDFLAGS=-Wl,--gc-sections,-Map=$(TARGET).map $(LIBS)

SOURCES= \
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
	uuid.c \

VPATH=.:../bluez/src/shared:../bluez/src:../bluez/lib

OBJECTS=$(SOURCES:.c=.o)
TARGET=wallclock
DEPS=$(OBJECTS:.o=.d)
ECHO=@

STYLEFLAGS = \
	--style=linux --indent=spaces=4 --indent-labels \
	--pad-oper --pad-header --unpad-paren --align-pointer=name \
	--align-reference=name --remove-brackets \
	--remove-comment-prefix --max-code-length=120 \
	--suffix=none --lineend=linux

.PHONY: clean install all style

all: $(TARGET) makefile

-include $(DEPS)

$(TARGET): $(OBJECTS) makefile
	@echo "CCLD $(OBJECTS)"
	$(ECHO)$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c makefile
	@echo "CC $<"
	$(ECHO)$(CC) $(CFLAGS) -MMD -c $< -o $@

clean:
	@rm -rf $(OBJECTS) $(DEPS) *.map $(TARGET)

install: $(TARGET)
	sudo cp powernet421.mib /var/lib/mibs/ietf
	sudo cp $(TARGET) /usr/libexec/bluetooth

style:
	@astyle $(STYLEFLAGS) $(wildcard *.c) $(wildcard *.h)

