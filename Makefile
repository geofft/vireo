CFLAGS = -ggdb3 -Wall -Wextra -Werror

targets = libvireo.so $(basename $(wildcard examples/*.c))
all: $(targets)

libvireo.so: vireo.c vireo.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $<

examples/%: examples/%.c vireo.h | libvireo.so
	$(CC) $(CFLAGS) -I. -o $@ $< -L. -lvireo -Wl,-rpath,\$$ORIGIN/..

clean:
	rm -f $(targets)
.PHONY: clean
