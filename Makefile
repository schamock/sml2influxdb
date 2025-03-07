CC = gcc
CFLAGS = -Wall -Wpedantic -Wextra -Werror

TARGETDIR := bin
TARGET := $(TARGETDIR)/sml2influxdb
SRCDIR := src
BUILDDIR := build
SRCEXT := c
INC := -I include
SOURCES := $(shell find $(SRCDIR) -type f -name '*.$(SRCEXT)')
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
LIBS := -lm -lcurl

.PHONY: clean all install systemd help

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(TARGETDIR)
	$(CC) $^ -o $(TARGET) $(CFLAGS) $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	-rm -f $(OBJECTS) $(TARGET)

install: $(TARGET)
ifneq ($(shell id -u),0)
	@echo "You must be root to perform this action."
	@exit 1
else
	install -m 744 $(TARGET) /usr/local/bin
endif

systemd-install: install
ifneq ($(shell id -u),0)
	@echo "You must be root to perform this action."
	@exit 1
else
	install -m 644 accessory/sml2influxdb.serice /etc/systemd/system
	systemctl daemon-reload
	@echo "You should run 'systemctl enable sml2influxdb.service' and 'systemctl start sml2influxdb.serivce' to start the service"
endif

help:
	@echo "You have the following options:"
	@echo "$$ make                 --> build the project"
	@echo "$$ make install         --> copy the binary to /usr/local/bin; must be run as root/via sudo"
	@echo "$$ make clean           --> remove the binary and objectfiles from the projectdirectory"
	@echo "$$ make systemd-install --> copy a servicefile to /etc/systemd/system and reload systemd to notice the new service"