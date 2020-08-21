#ifndef _BUILDROOMS_H
#define _BUILDROOMS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>	//get_pid()
#include <sys/stat.h>	//mkdir()
#include <string.h>

//#include <fcntl.h>
//#include <dirent.h>

#define NUM_ROOMS 7
#define NUM_CONNECTIONS 6

struct room {
	char* name;
	char* type;
	int num_connections;
	struct room* outbound_connections[NUM_CONNECTIONS];
};

char ROOM_NAMES[10][15] = { 
		"Squirtle_Room", 
		"Scyther_Room", 
		"Venusaur_Room",
	   	"Snorlax_Room", 
		"Gengar_Room", 
		"Arcanine_Room", 
		"Jolteon_Room", 
		"Seadra_Room", 
	   	"Porygon_Room", 
		"Dratini_Room" 
}; 

//Functions to connect rooms
int is_graph_full(struct room[NUM_ROOMS]);
void add_random_connection(struct room[NUM_ROOMS]);
struct room* get_random_room(struct room[NUM_ROOMS]);
int can_add_connection_from(struct room x);
int connection_already_exists(struct room x, struct room y);
int is_same_room(struct room x, struct room y);
void connect_room(struct room* x, struct room* y);

//Functions to build files
struct room* allocate_rooms();
void generate_room_names_types (struct room*);
void deallocate(struct room*);
void build_files(struct room*);
void write_file_info(FILE*, struct room);
#endif
