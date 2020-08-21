CC=gcc
CFLAGS := -Wall -g -std=c99
CLIBS := -lpthread

GAME_EXE=adventure
BUILD_EXE=build_rooms

all: $(GAME_EXE) $(BUILD_EXE)

$(GAME_EXE): adventure.h adventure.c singlylist.h singlylist.c get_input.h get_input.c
	$(CC) $(CFLAGS) $(CLIBS) -o $@ adventure.c singlylist.c get_input.c

$(BUILD_EXE): buildrooms.h buildrooms.c
	$(CC) $(CFLAGS) -o $@ buildrooms.c

clean:
	rm -f $(BUILD_EXE)
	rm -f $(GAME_EXE)
	rm -rf adventure.rooms.*