CC = gcc
CFLAGS = -Wall -Wpedantic -Wextra

TARGETDIR := bin
TARGET := $(TARGETDIR)/easymeter
SRCDIR := src
BUILDDIR := build
SRCEXT := c
INC := -I include
SOURCES := $(shell find $(SRCDIR) -type f -name '*.$(SRCEXT)')
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

.PHONY: clean all

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(TARGETDIR)
	$(CC) $^ -o $(TARGETDIR)/$(TARGET) $(CFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	-rm -f $(OBJECTS) $(TARGET)
