CC=gcc
CFLAGS=-c -g -Wall -std=c99
LDFLAGS=-pthread
 
SOURCES=nivel1.c nivel2.c nivel3.c nivel4.c nivel5.c nivel6.c
LIBRARIES=
INCLUDES=
PROGRAMS= nivel1 nivel2 nivel3 nivel4 nivel5 nivel6
OBJS=$(SOURCES:.c=.o)
 
all: $(OBJS) $(PROGRAMS)
 
#$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
#   $(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $@
 
 
av3: av3.o $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIES) $< -o $@
 
#reader: reader.o $(LIBRARIES) $(INCLUDES)
#	$(CC) $(LDFLAGS) $(LIBRARIES) $< -o $@
 
%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<
 
.PHONY: clean
clean:
	rm -rf *.o *~ $(PROGRAMS)
