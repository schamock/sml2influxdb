CC := gcc
CFLAGS := -Wall -Wpedantic -Wextra -Werror

BINARY := sml2influxdb
TARGETDIR := bin
TARGET := $(TARGETDIR)/$(BINARY)
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
	@echo "You must run 'make install' via sudo to perform this action!"
	@exit 1
else
	install -m 754 $(TARGET) /usr/local/bin
	chown $$SUDO_USER /usr/local/bin/$(BINARY)
endif

systemd-install: install
ifneq ($(shell id -u),0)
	@echo "You must be root to perform this action!"
	@exit 1
else
	@echo "Make sure, that the 'user' parameter in accessory/sml2influxdb.serivce is set to an apropriate value !!!"
	install -m 644 accessory/sml2influxdb.service /etc/systemd/system
	systemctl daemon-reload
	@echo "You should run 'systemctl enable sml2influxdb.service' and 'systemctl start sml2influxdb.serivce' to start the service."
endif

help:
	@echo "You have the following options:"
	@echo "$$ make                 --> build the project"
	@echo "$$ make install         --> copy the binary to /usr/local/bin; must be run as root/via sudo"
	@echo "$$ make clean           --> remove the binary and objectfiles from the projectdirectory"
	@echo "$$ make systemd-install --> copy a servicefile to /etc/systemd/system and reload systemd to notice the new service"