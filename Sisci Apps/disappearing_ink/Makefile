.PHONY: clean

LFLAGS= -L/opt/DIS/lib64 -lsisci
INCLUDES = -I/opt/DIS/include -I/opt/DIS/include/dis

FILES = disappearing_ink

WARNINGS = -W -Wall -Wextra -Wpedantic

all: $(FILES)

disappearing_ink: disappearing_ink.c
	gcc $(WARNINGS) $(INCLUDES) $< -o $@ $(LFLAGS)

clean:
	rm -f $(FILES) *.o
