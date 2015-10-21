CC = gcc
CFLAGS = -static

nginx_reloader : main.c
	$(CC) $(CFLAGS) -o $@ $<
