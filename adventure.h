#ifndef _ADVENTURE_H_
#define _ADVENTURE_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <assert.h>

#include "singlylist.h"

#define NUM_CONNECTIONS 6
#define NUM_ROOMS 7
#define FILE_NAME "currentTime.txt"
#define ROOM_INFO_LENGTH 16

// commands
#define COM_TIME -2

struct room {
	char* name;
	char* type;
	int num_connections;
	struct room* outbound_connections[NUM_CONNECTIONS];
};

// setup
char** get_file_paths();
char* read_my_dir();
struct room* allocate_rooms();
void read_files(struct room*, char**);
void attach_connections(struct room*, char**);

// game interactions
struct room* get_start(struct room*);
void* get_current_time();
char* read_time_file();
int check_input(char*, struct room*);
void print_room_info(struct room*);
struct room* get_room(struct room*, char*);
void game(int*, struct node**, struct room**, pthread_t*, pthread_attr_t*);
void clear_screen();
void print_game_info(char*, struct room*);

// cleanup
void de_allocate_filepaths(char**);
void de_allocate_rooms(struct room*);

#endif
