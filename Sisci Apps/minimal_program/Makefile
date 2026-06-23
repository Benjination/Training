
.PHONY: clean

LFLAGS= -L/opt/DIS/lib64 -lsisci
INCLUDES = -I/opt/DIS/include -I/opt/DIS/include/dis

FILES = simple

WARNINGS = -W -Wall -Wextra -Wpedantic

all: $(FILES)

simple: simple.c
	gcc $(WARNINGS) $(INCLUDES) $< -o $@ $(LFLAGS)

clean:
	rm -f $(FILES) *.o