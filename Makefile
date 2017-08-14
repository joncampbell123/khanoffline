
TARGETS=arith1

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

arith1: arith1.o
	gcc -o $@ $<

.c.o:
	gcc -c -o $@ $<

