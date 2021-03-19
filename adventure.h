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

#define COM_TIME -2

struct room {
    char* name;
    char* type;
    int num_connections;
    struct room* outbound_connections[NUM_CONNECTIONS];
};

struct game {
    int step_count;
    struct node* user_path;
    struct room* rooms_array;
};

// setup
int init_game(struct game**);
struct game* allocate_game();
char** get_file_paths();
char* read_my_dir();
struct room* allocate_rooms();
void read_files(struct game**, char**);
void attach_connections(struct game**, char**);

// game interactions
struct room* get_start(struct game**);
void* get_current_time();
char* read_time_file();
int check_input(char*, struct room*);
void print_room_info(struct room*);
struct room* get_room(struct game**, char*);
int run_game(struct game**);
void print_game_info(char*, struct room*);

// cleanup
void de_allocate_game(struct game**);
void de_allocate_filepaths(char**);
void de_allocate_rooms(struct room*);

#endif
