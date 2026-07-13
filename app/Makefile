CC = gcc
WINDOWSCC = x86_64-w64-mingw32-gcc

CFLAGS = -Wall -Wextra -g -Iinclude
# Keep Windows flags clean from Linux-specific pkg-config values
WINDOWSCFLAGS = -Wall -Wextra -g -Iinclude 

EXE = keylogger
WINEXE = keylogger.exe

BUILDDIR = build
WINBUILDDIR = build/windows
SRCDIR = src

SRC = $(wildcard $(SRCDIR)/*.c)

OBJ = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRC))
WINOBJ = $(patsubst $(SRCDIR)/%.c,$(WINBUILDDIR)/%.o,$(SRC))

XKB_CFLAGS := $(shell pkg-config --cflags xkbcommon)
XKB_LIBS   := $(shell pkg-config --libs xkbcommon)

# Only append to Linux CFLAGS
CFLAGS += $(XKB_CFLAGS)

all: $(BUILDDIR)/$(EXE)

windows: $(WINBUILDDIR)/$(WINEXE)

$(BUILDDIR)/$(EXE): $(OBJ)
	$(CC) $^ -o $@ $(XKB_LIBS)

# ADDED -luser32 HERE to link Windows API libraries
$(WINBUILDDIR)/$(WINEXE): $(WINOBJ)
	$(WINDOWSCC) $^ -o $@ -luser32

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(WINBUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(WINBUILDDIR)
	$(WINDOWSCC) $(WINDOWSCFLAGS) -c $< -o $@

clean:
	rm -rf build


run: $(BUILDDIR)/$(EXE)
	$< /dev/input/event0 ./log.txt

run_windows: $(WINBUILDDIR)/$(WINEXE)
	wine $< ./log.txt

# CC = gcc
# WINDOWSCC = x86_64-w64-mingw32-gcc

# CFLAGS = -Wall -Wextra -g -Iinclude
# WINDOWSCFLAGS = $(CFLAGS)

# EXE = keylogger
# WINEXE = keylogger.exe

# BUILDDIR = build
# WINBUILDDIR = build/windows
# SRCDIR = src

# SRC = $(wildcard $(SRCDIR)/*.c)

# OBJ = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRC))
# WINOBJ = $(patsubst $(SRCDIR)/%.c,$(WINBUILDDIR)/%.o,$(SRC))

# XKB_CFLAGS := $(shell pkg-config --cflags xkbcommon)
# XKB_LIBS   := $(shell pkg-config --libs xkbcommon)

# CFLAGS += $(XKB_CFLAGS)

# all: $(BUILDDIR)/$(EXE)

# windows: $(WINBUILDDIR)/$(WINEXE)

# $(BUILDDIR)/$(EXE): $(OBJ)
# 	$(CC) $^ -o $@ $(XKB_LIBS)

# $(WINBUILDDIR)/$(WINEXE): $(WINOBJ)
# 	$(WINDOWSCC) $^ -o $@

# $(BUILDDIR)/%.o: $(SRCDIR)/%.c
# 	mkdir -p $(BUILDDIR)
# 	$(CC) $(CFLAGS) -c $< -o $@

# $(WINBUILDDIR)/%.o: $(SRCDIR)/%.c
# 	mkdir -p $(WINBUILDDIR)
# 	$(WINDOWSCC) $(WINDOWSCFLAGS) -c $< -o $@

# clean:
# 	rm -rf build

# run:
# 	$(BUILDDIR)/$(EXE) /dev/input/event0

# log:
# 	$(BUILDDIR)/$(EXE) /dev/input/event0 ./log.txt

# debug:
# 	gdb $(BUILDDIR)/$(EXE) /dev/input/event0