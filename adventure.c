#ifndef _ADVENTURE_C_
#define _ADVENTURE_C_

#include "adventure.h"
#include "get_input.h"

pthread_mutex_t mutex1;

// for printing error message if bad input
int input_err = 0;
int start = 0;

int main(int argc, char* argv[]) {
    // filepaths for room info files
    char** filepath_array = NULL;
    // game struct for game components
    struct game* game = NULL;

    //search for directory, verify files
    if (!(filepath_array = get_file_paths())) {
        perror("Error reading files.");
        exit(1);
    }

    if (!(game = allocate_game())) {
        perror("Error initializing game");
        return 1;
    }

	// build room structs from files
	read_files(&game, filepath_array);
	de_allocate_filepaths(filepath_array);

	// initialize mutex lock
	if (pthread_mutex_init(&mutex1, NULL) != 0) {
		perror("Error initializing mutex");
		return 1;
	}

	// get lock
	pthread_mutex_lock(&mutex1);

	// pthread
	pthread_t my_thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, 1);

	// run the game !
	run_game(&game, &my_thread, &attr);

	//print contents of user_path up to step_count
	print_list(game->user_path, game->step_count);

	//free memory used
    de_allocate_game(&game);

	//thread cleanup
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex1);

	return 0;
}

/* FUNCTIONS */

/******************************************************************************
 *      Description: handles game logic
 *       Parameters: address of game struct pointer, pthread_t* address of thread struct,
 *                   pthread_attr_t* address of thread attribute struct
 *          Returns: none
 *   Pre-conditions: game components initialized
 *  Post-conditions: user path added to linked list of rooms visited
 *                   and step counter incremented
******************************************************************************/
void
run_game (struct game** game, pthread_t* my_thread, pthread_attr_t* attr)
{
    // user input
    char* user_input = NULL;
    int buffer_length = 0, input_length = 0;

    // time
    char* time_string = NULL;

    //get start_room and set to current room
    struct room* current_room = get_start(game);
    add_node(&(*game)->user_path, current_room->name);

    /*Game Loop*/
    while (strcmp(current_room->type, "END_ROOM")) {
        print_game_info(time_string, current_room);
        if (time_string) {
            free(time_string);
            time_string = NULL;
        }

        if ((input_length = get_line(&user_input, &buffer_length, stdin)) < 0) {
            perror("Error reading input");
            de_allocate_game(game);
            exit(1);
        }

        // check input
        int result = check_input(user_input, current_room);

        if (result == COM_TIME) {
            if (pthread_create(my_thread, attr, get_current_time, NULL) != 0) {
                perror("Error creating thread");
                free(user_input);
                de_allocate_game(game);
                exit(1);
            }

            // release lock
            pthread_mutex_unlock(&mutex1);
            pthread_join(*my_thread, NULL);

            time_string = read_time_file();
            if (!time_string) {
                free(user_input);
                de_allocate_game(game);
                exit(1);
            }

            pthread_mutex_lock(&mutex1);
        }
        else if (result >= 0) {
            current_room = get_room(game, user_input);
            //add room name to user path
            (*game)->step_count++;
            add_node(&(*game)->user_path, current_room->name);
        }
        else {
            input_err = 1;
        }

        if (user_input)
            memset(user_input, '\0', input_length);
    }
    /*End Game Loop*/
    free(user_input);
}

/******************************************************************************
 *     Description: Prints game info to same lines on console
 *      Parameters: receives c_string and struct room pointer
 *         Returns: none
 *  Pre-conditions: struct room is not NULL
 * Post-conditions: game information printed to screen
******************************************************************************/
void
print_game_info(char* time_string, struct room* r) {
    if (!start) {
        start = 1;
    }
    else {
        printf("\e[10A\e[0J");
    }

    printf("\n");
    if (time_string) {
        printf("\e[0K\t%s\n", time_string);
    } else {
        printf("\e[0K\n");
    }
    printf("\n");

    print_room_info(r);

    if (input_err) {
        printf("\e[0KHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
        input_err = 0;
    } else {
        printf("\e[0K\n");
    }

    printf("\e[0KWHERE TO? > ");
}

/******************************************************************************
 *	   Description: Prints information for current room
 *	    Parameters: receives address to current room
 *	       Returns: none
 *	Pre-conditions: current_room is not NULL
 *     Post-conditions: room information printed to screen
******************************************************************************/
void
print_room_info (struct room* current_room) {
    int line_count = 3;

    printf("\e[0KCURRENT LOCATION:\t%s\n", current_room->name);
    printf("\e[0KPOSSIBLE CONNECTIONS:\n\e[0K\t");

    for (int i = 0; i < current_room->num_connections; ++i) {
        if (i && !(i % 3)) {
            printf("\n\t");
            line_count++;
        }
        printf("%s   ", current_room->outbound_connections[i]->name);
    }

    while (line_count < 6) {
        printf("\n");
        line_count++;
    }
}

/******************************************************************************
 *     Description: allocates memory on heap for game struct
 *      Parameters: none
 *         Returns: struct game pointer
 *  Pre-conditions: enough memory for game struct
 * Post-conditions: memory reserved for struct game and fields
******************************************************************************/
struct
game* allocate_game() {
    struct game* game = (struct game*) malloc(sizeof(struct game));
    assert(game);

    game->step_count = 0;
    game->user_path = NULL;
    game->rooms_array = allocate_rooms();
    assert(game->rooms_array);

    return game;
}

/******************************************************************************
 *     Description: allocates memory on heap for an array of struct room
 *      Parameters: none
 *         Returns: struct room[NUM_ROOMS]
 *  Pre-conditions: enough memory for sizeof(struct room)*NUM_ROOMS
 * Post-conditions: memory reserved for struct room[NUM_ROOMS]
******************************************************************************/
struct
room* allocate_rooms() {
    struct room* temp_array = (struct room*) malloc(NUM_ROOMS * sizeof(struct room));
    assert(temp_array);

    for (int i = 0; i < NUM_ROOMS; i++) {
        temp_array[i].name = (char*) malloc(ROOM_INFO_LENGTH * sizeof(char));
        memset(temp_array[i].name, '\0', ROOM_INFO_LENGTH);
        temp_array[i].type = (char*) malloc(ROOM_INFO_LENGTH * sizeof(char));
        memset(temp_array[i].type, '\0', ROOM_INFO_LENGTH);
        temp_array[i].num_connections = 0;

        for (int j=0; j < NUM_CONNECTIONS; j++){
            temp_array[i].outbound_connections[j] = NULL;
        }
    }

    return temp_array;
}

/******************************************************************************
 *     Description: Searches array of rooms for room with given string
 *      Parameters: address of game struct pointer and c-style string
 *         Returns: room with desired name
 *  Pre-conditions: room_name exists in the rooms_array
 * Post-conditions: returns a struct room
******************************************************************************/
struct
room* get_room(struct game** game, char* room_name) {
    for (int i = 0; i < NUM_ROOMS; ++i){
        if (!(strcmp((*game)->rooms_array[i].name, room_name))) {
            return &((*game)->rooms_array[i]);
        }
    }
    return NULL;
}

/******************************************************************************
 *      Description: Searches array of rooms for the room with type START_ROOM
 *       Parameters: address of game struct pointer
 *          Returns: struct room or NULL
 *   Pre-conditions: rooms_array is not empty
 *  Post-conditions: room struct is returned
******************************************************************************/
struct
room* get_start(struct game** game){
    for (int i = 0; i < NUM_ROOMS; ++i) {
        if (!(strcmp((*game)->rooms_array[i].type, "START_ROOM"))) {
            return &((*game)->rooms_array[i]);
        }
    }
    return NULL;
}

/******************************************************************************
 *	   Description: thread start routine, prints current time to a file
 *	    Parameters: none
 *	       Returns: none
 *	Pre-conditions: none
 *     Post-conditions: new file exists that
******************************************************************************/
void*
get_current_time(){
    time_t rawtime;
    struct tm *info;
    char time_string[64];
    memset(time_string, '\0', 64);
    FILE *fptr;
    char filename[16];
    memset(filename, '\0', 16);
    strncpy(filename, FILE_NAME, 16);
    pthread_mutex_lock(&mutex1);

    //get current time and store it in time_string
    time(&rawtime);
    info = localtime(&rawtime);
    strftime(time_string, 64, "%I:%M%p, %A, %B %d, %Y", info);

    //print time_string to file
    fptr = fopen(filename, "w");
    fprintf(fptr, "%s\n", time_string);
    fclose(fptr);
    pthread_mutex_unlock(&mutex1);

    pthread_exit(NULL);
}

/******************************************************************************
 *     Description: reads a line from file that holds the current time
 *      Parameters: none
 *         Returns: char* time
 *  Pre-conditions: file exists and is readable
 * Post-conditions: current time printed to screen
******************************************************************************/
char*
read_time_file(){
    FILE *fptr;
    char* time_string = NULL;
    char filename[16];
    int buffer_length = 0;

    memset(filename, '\0', 16);
    strncpy(filename, FILE_NAME, 16);

    // keep trying until file can be opened
    while (!(fptr = fopen(filename, "r"))) ;

    // read time stamp from file
    if (get_line(&time_string, &buffer_length, fptr) < 0) {
        perror("Error reading time file");
        fclose(fptr);
        return NULL;
    }

    fclose(fptr);

    return time_string;
}
/******************************************************************************
 *     Description: checks user input to match a room name in the current
 *                  room's outboundConnections
 *      Parameters: Receives user input and pointer to struct room
 *         Returns: if user_input doesn't match a outbound room name return -1
 *                  otherwise returns index of room
 *  Pre-conditions: user_input is not empty, current_room is not empty
 * Post-conditions: int is returned
******************************************************************************/
int
check_input(char* user_input, struct room* current_room){
    if (!strcmp(user_input, "time"))
        return COM_TIME;

    for (int i = 0; i < current_room->num_connections; i++) {
        if(!strcmp(user_input, current_room->outbound_connections[i]->name))
            return i;
    }

    // input does not match any room
    return -1;
}

/******************************************************************************
 *     Description: gets path to room file
 *      Parameters: none
 *         Returns: array of strings (char*)
 *  Pre-conditions: files exist
 * Post-conditions: 2d array is returned to calling function
******************************************************************************/
char**
get_file_paths(){
    char** filepath_array;
    char file_suffix[6] = "_Room";
    char* my_dir = NULL;
    DIR* dir_ptr = NULL;

    filepath_array = (char**) calloc(NUM_ROOMS, sizeof(char*));
    assert(filepath_array);

    for (int  i = 0; i < NUM_ROOMS; i++){
        filepath_array[i] = calloc(64, sizeof(char));
        memset(filepath_array[i], '\0', 64);
    }

    my_dir = read_my_dir();

    if ((dir_ptr = opendir(my_dir)) > 0) {
        int index = 0;
        struct dirent* file_in_dir;

        while ((file_in_dir = readdir(dir_ptr))) {
            if (strstr(file_in_dir->d_name, file_suffix)) {
                sprintf(filepath_array[index], "%s/%s", my_dir, file_in_dir->d_name);
                index++;
            }
        }
    }

	closedir(dir_ptr);
	free(my_dir);

    return filepath_array[0][0] ? filepath_array : NULL;
}

/******************************************************************************
 *     Description: searches for directory with onid username
 *      Parameters: none
 *         Returns: receives array of char holding directory name
 *  Pre-conditions: directory exists
 * Post-conditions: returns string containing newest directory
******************************************************************************/
char*
read_my_dir(){
    int newest_dir_time = -1;
    char dir_prefix[17] = "adventure.rooms.";

    char* newest_dir_name;
    newest_dir_name = (char*) malloc(64 * sizeof(char));
    memset(newest_dir_name, '\0', 64);

    DIR* dir_ptr;
    struct dirent* file_in_dir;
    struct stat dir_attributes;

    dir_ptr = opendir(".");

    if (dir_ptr > 0 ) {
        // iterate over files in current directory
        while ((file_in_dir = readdir(dir_ptr))) {
            // check if current file begins with dir_prefix
            if (strstr(file_in_dir->d_name, dir_prefix)) {
                // get file attributes of directory as a struct stat
                stat(file_in_dir->d_name, &dir_attributes);

                // compare modified time of file to current newest time
                if ((int)dir_attributes.st_mtime > newest_dir_time) {
                    newest_dir_time = (int)dir_attributes.st_mtime;
                    memset(newest_dir_name, '\0', 64);
                    strncpy(newest_dir_name, file_in_dir->d_name, 64);
                }//end of if file newer than previous newest file
            }//end of if filename contains my str

        }//end while loop

        if ( !newest_dir_name[0] ) {
            printf("Can not find correct directory!\n");
            free(newest_dir_name);
            exit(1);
        }
    }//end if dirPtr

    closedir(dir_ptr);

	return newest_dir_name;
}

/******************************************************************************
 *     Description: Attaches the connections to each room struct
 *      Parameters: struct game address pointer and 2d array holding filepaths
 *         Returns: none
 *  Pre-conditions: rooms_array is filled with name and type for each room
 * Post-conditions: outboundConnections array for each room contains the
 *                  correct address for connected rooms
******************************************************************************/
void
attach_connections(struct game** game, char** filepath_array) {
    FILE* fptr;

    for (int i = 0; i < NUM_ROOMS; ++i) {
        int connection_index = 0;
        char arg1[ROOM_INFO_LENGTH], arg2[ROOM_INFO_LENGTH], arg3[ROOM_INFO_LENGTH];
        memset(arg1, '\0', ROOM_INFO_LENGTH);
        memset(arg2, '\0', ROOM_INFO_LENGTH);
        memset(arg3, '\0', ROOM_INFO_LENGTH);

        if (!(fptr = fopen(filepath_array[i], "r"))) {
            perror("Error reading file.");
            exit(1);
        }

        // read connections and add to room struct
        while((fscanf(fptr, "%s %s %s", arg1, arg2, arg3)) != EOF){
            if (!strcmp(arg1, "CONNECTION")) {
                struct room* connection = get_room(game, arg3);
                (*game)->rooms_array[i].outbound_connections[connection_index] = connection;
                (*game)->rooms_array[i].num_connections++;
                connection_index++;
            }

            memset(arg1, '\0', ROOM_INFO_LENGTH);
            memset(arg2, '\0', ROOM_INFO_LENGTH);
            memset(arg3, '\0', ROOM_INFO_LENGTH);
        }

        fclose(fptr);
    }
}

/******************************************************************************
 *	   Description: reads files within my directory and attaches appropriate
 *	   		data to corresponding struct
 *	    Parameters: receives rooms_array and 2D array of char containing filepaths
 *	       Returns: none
 *	Pre-conditions: rooms_array and filepath_array are not null,
 *			files exist and are readable
 *     Post-conditions: each room in rooms_array is updated with information
 *     			contained in their files
******************************************************************************/
void
read_files(struct game** game, char** filepath_array){
    FILE* fptr;

    for (int i = 0; i < NUM_ROOMS; ++i) {
        char arg1[ROOM_INFO_LENGTH], arg2[ROOM_INFO_LENGTH], arg3[ROOM_INFO_LENGTH];
        memset(arg1, '\0', ROOM_INFO_LENGTH);
        memset(arg2, '\0', ROOM_INFO_LENGTH);
        memset(arg3, '\0', ROOM_INFO_LENGTH);

        if(!(fptr = fopen(filepath_array[i], "r"))) {
            perror("Error reading file.");
            exit(1);
        }

        // read through room file
        while ((fscanf(fptr, "%s %s %s", arg1, arg2, arg3)) != EOF){
            if (!strcmp(arg1, "ROOM")) {
                if (!strcmp(arg2, "TYPE:")) {
                    strncpy((*game)->rooms_array[i].type, arg3, ROOM_INFO_LENGTH);
                }
                else {
                    strncpy((*game)->rooms_array[i].name, arg3, ROOM_INFO_LENGTH);
                }
            }

            memset(arg1, '\0', ROOM_INFO_LENGTH);
            memset(arg2, '\0', ROOM_INFO_LENGTH);
            memset(arg3, '\0', ROOM_INFO_LENGTH);
        }
        assert((*game)->rooms_array[i].name);
        assert((*game)->rooms_array[i].type);

        fclose(fptr);
    }

    attach_connections(game, filepath_array);
}

/******************************************************************************
 *	   Description: Frees memory allocated for 2D array of chars containing
 *	   		filepaths
 *	    Parameters: receives 2D array of char
 *	       Returns: none
 *	Pre-conditions: filepaths_array is not NULL
 *     Post-conditions: memory allocated for filepaths_array is free'd
******************************************************************************/
void
de_allocate_filepaths(char** filepaths_array) {
    if (filepaths_array) {
        for (int i = (NUM_ROOMS-1); i >= 0; i--) {
            free(filepaths_array[i]);
            filepaths_array[i] = NULL;
        }

        free(filepaths_array);
        filepaths_array = NULL;
    }
}

/******************************************************************************
 *      Description: Frees memory allocated for game components
 *       Parameters: receives pointer to game struct
 *          Returns: none
 *   Pre-conditions: filepaths_array is not NULL
 *  Post-conditions: memory allocated for filepaths_array is free'd
******************************************************************************/
void
de_allocate_game(struct game** game) {
    if (game) {
        de_allocate_rooms((*game)->rooms_array);
        free_list(&((*game)->user_path));
        free((*game));
        game = NULL;
    }
}

/******************************************************************************
 *	   Description: Frees memory allocated for array of struct room
 *	    Parameters: receives array of struct room
 *	       Returns: none
 *	Pre-conditions: rooms_array is not NULL
 *     Post-conditions: memory allocated for rooms_array is free'd
******************************************************************************/
void
de_allocate_rooms(struct room* rooms_array){
    if (rooms_array) {
        for (int i = 0; i < NUM_ROOMS; i++){
            free(rooms_array[i].name);
            rooms_array[i].name = NULL;
            free(rooms_array[i].type);
            rooms_array[i].type = NULL;
        }

    free(rooms_array);
    rooms_array = NULL;
    }
}

#endif
