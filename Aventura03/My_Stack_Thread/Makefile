CC=gcc
CFLAGS=-c -g -Wall -std=c99
LDFLAGS=-pthread
 
SOURCES=av3.c #reader.c
LIBRARIES=
INCLUDES=
PROGRAMS= av3 #reader
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
