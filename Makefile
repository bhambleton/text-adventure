CC=gcc
CFLAGS := -Wall -g -std=c99
CLIBS := -lpthread

GAME_EXE=adventure
BUILD_EXE=build_rooms

all: $(BUILD_EXE) $(GAME_EXE)

$(GAME_EXE): adventure.h adventure.c singlylist.h singlylist.c get_input.h get_input.c
	$(CC) $(CFLAGS) $(CLIBS) -o $@ adventure.c singlylist.c get_input.c

$(BUILD_EXE): buildrooms.h buildrooms.c
	$(CC) $(CFLAGS) -o $@ buildrooms.c

run: $(GAME_EXE) $(BUILD_EXE)
	./$(BUILD_EXE)
	./$(GAME_EXE)

clean:
	rm -f $(BUILD_EXE) $(GAME_EXE)
	rm -rf adventure.rooms.* $(BUILD_EXE).dSYM/ $(GAME_EXE).dSYM/
	rm -f currentTime.txt
