CC = gcc
CFLAGS = -Wall -Wpedantic -Wextra -Werror

TARGETDIR := bin
TARGET := $(TARGETDIR)/easymeter
SRCDIR := src
BUILDDIR := build
SRCEXT := c
INC := -I include
SOURCES := $(shell find $(SRCDIR) -type f -name '*.$(SRCEXT)')
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
LIBS := -lm -lcurl

.PHONY: clean all

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(TARGETDIR)
	$(CC) $^ -o $(TARGET) $(CFLAGS) $(LIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	-rm -f $(OBJECTS) $(TARGET)
