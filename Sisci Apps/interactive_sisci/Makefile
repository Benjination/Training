.PHONY: clean

INCLUDES = -I/opt/DIS/include -I/opt/DIS/include/dis # -I$/opt/DIS/src/include

LFLAGS = -L/opt/DIS/lib64 -lsisci
# FLAGS = -std=c11
FLAGS = -std=gnu89 -Wall -Wextra -O2
# FLAGS = -Wall -Wextra -Wunused-result -O2


FILES = interactive_sisci

interactive_sisci: interactive_sisci.c
	gcc $(FLAGS) $(INCLUDES) $< -o $@ $(LFLAGS)

clean:
	rm -f $(FILES) *.o
