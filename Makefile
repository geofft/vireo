CFLAGS = -ggdb3 -Wall -Wextra -Werror

targets = libvireo.so
all: $(targets)

libvireo.so: vireo.c vireo.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<

clean:
	rm -f $(targets)
.PHONY: clean
