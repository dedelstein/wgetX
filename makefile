LINK_TARGET = wgetX

OBJS = \
	url.o \
	wgetX.o

CFLAGS = -Wall -g

REBUILDABLES = $(OBJS) $(LINK_TARGET) test_url

all: $(LINK_TARGET)

wgetX: wgetX.o url.o
	$(CC) -o wgetX wgetX.o url.o $(CFLAGS)

test_url: test_url.o url.o
	$(CC) -o test_url test_url.o url.o $(CFLAGS)

clean:
	rm -f $(REBUILDABLES)
