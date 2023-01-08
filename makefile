
all: downloader

.PHONY: all clean

downloader: Client.c
	gcc -o downloader Client.c functions.c conn.c download.c config.h -pthread

clean:
	rm -r downloader

