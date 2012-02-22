
all: run

run: spawn.cpp
	g++ -Wall -Wno-write-strings `pkg-config --libs --cflags glib-2.0` -o run spawn.cpp

clean:
	rm -f run

