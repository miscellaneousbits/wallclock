CC=gcc

INSTALLDIR=/usr/local/bin
BLUEZDIR=../bluez-5.50

INCLUDES= \
	-I/usr/include/dbus-1.0 \
	-I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include \
	-I/usr/include/glib-2.0 \
	-I$(BLUEZDIR)

LIBS=-lm -lbluetooth -lsystemd -lpthread

COPTFLAG=-O3
#COPTFLAG=-g

CFLAGS=$(COPTFLAG) -Wall -ffunction-sections -fdata-sections $(INCLUDES)
LDFLAGS=$(COPTFLAG) -Wl,--gc-sections,-Map=$(TARGET).map

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

VPATH=.:$(BLUEZDIR)/src/shared:$(BLUEZDIR)/src:$(BLUEZDIR)/lib

OBJECTS=$(SOURCES:.c=.o)
TARGET=wallclock
DEPS=$(OBJECTS:.o=.d)
ECHO=@
#ECHO=

STYLEFLAGS = \
	--style=linux --indent=spaces=4 --indent-labels \
	--pad-oper --pad-header --unpad-paren --align-pointer=name \
	--align-reference=name --remove-brackets \
	--remove-comment-prefix --max-code-length=120 \
	--suffix=none --lineend=linux

.PHONY: clean install all style

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

style:
	@astyle $(STYLEFLAGS) $(wildcard *.c) $(wildcard *.h)

