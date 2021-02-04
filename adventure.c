#ifndef _ADVENTURE_C_
#define _ADVENTURE_C_

#include "adventure.h"
#include "get_input.h"

pthread_mutex_t mutex1;
int print_check = 0;

int main() {
	//user path linked list
	struct node* user_path = NULL;
    int step_count = 0;
	
	//search for directory, verify files
	char** filepath_array = get_file_paths();
	if (filepath_array == NULL) {
		perror("Error reading files.");
		exit(1);
	}

	struct room* rooms_array = allocate_rooms();

	// build room structs from files
	read_files(rooms_array, filepath_array);
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
    game(&step_count, &user_path, &rooms_array, &my_thread, &attr);

	//print contents of user_path up to step_count
	print_list(user_path, step_count);

	//free memory used
	free_list(&user_path);
	de_allocate_rooms(rooms_array);

	//thread cleanup
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex1);
	pthread_exit(0);

	return 0;
}

/* FUNCTIONS */

/*
 * TODO: fix how this is done so it is not broken
 *          when the time is printed.
 *
 *      Currently a super clunky use of ANSI escape sequences
 *      for cursor movement :D
 *
 * */
void 
clear_screen() {
    if (print_check == 2) {
		printf("\e[3F\e[J"); // move cursor up 3 lines and clear terminal below
		print_check--;
	}
	else if (print_check == 1) {
		printf("\e[4F\e[J"); // move cursor up 4 lines and clear terminal below
	    print_check = 0;
	}
	else if (print_check > 2) {
		printf("\e[5F\e[J");
		print_check -= 2;
	}
	else {
		printf("\e[3F\e[J");
	}
}


/******************************************************************************
 *	    Description: handles game logic
 *	     Parameters: int* step counter, struct node** address of pointer to linked list,
 *	                    struct room** address of array of struct room,
 *	                    pthread_t* address of thread, pthread_attr_t* address of thread attribute struct
 *	        Returns: none
 *	 Pre-conditions: game components initialized
 *  Post-conditions: user path added to linked list of rooms visited
 *                   and step counter incremented
******************************************************************************/
void 
game (int* step_count, struct node** user_path, struct room** rooms_array, 
        pthread_t* my_thread, pthread_attr_t* attr)
{
	char* user_input = NULL;
	int buffer_length = 0, input_length = 0;
    
    //get start_room and set to current room
	struct room* current_room = get_start(*rooms_array);
	add_node(user_path, current_room->name);

	//add some space above output
	printf("\n");
	
	/*Game Loop*/
    while (strcmp(current_room->type, "END_ROOM") != 0) {
		print_room_info(current_room);
		printf("WHERE TO? > ");
		input_length = get_line(&user_input, &buffer_length, stdin);

		// check input
		int result = check_input(user_input, current_room);

		if (result == COM_TIME) {
	        pthread_mutex_unlock(&mutex1);
            if (pthread_create(my_thread, attr, get_current_time, NULL) != 0) {
                perror("Error creating thread");
                free(user_input);
                free_list(user_path);
                de_allocate_rooms(*rooms_array);
                exit(1);
            }

            print_check += 2;
			pthread_join(*my_thread, NULL);
			pthread_mutex_lock(&mutex1);
            read_time_file();
		}
		else if (result >= 0) {
			current_room = get_room(*rooms_array, user_input);
			//add room name to user path
			(*step_count)++;
			add_node(user_path, current_room->name);
			clear_screen();
		}
		else {
			print_check += 2;
			clear_screen();
			puts("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.");
		}

		if (user_input != NULL)
			memset(user_input, '\0', input_length);
	}
	/*End Game Loop*/
	free(user_input);
}


/******************************************************************************
 *	   Description: allocates memory on the heap for an array of struct room
 *	    Parameters: none
 *	       Returns: struct room[NUM_ROOMS]
 *	Pre-conditions: enough memory for sizeof(struct room)*NUM_ROOMS
 *     Post-conditions: memory reserved for struct room[NUM_ROOMS]
******************************************************************************/
struct 
room* allocate_rooms() {
	struct room* temp_array = (struct room*) malloc(NUM_ROOMS * sizeof(struct room));
	for (int i = 0; i < NUM_ROOMS; i++) {
		temp_array[i].name = (char*) malloc(15 * sizeof(char));
		memset(temp_array[i].name, '\0', 15);
		temp_array[i].type = (char*) malloc(12 * sizeof(char));
		memset(temp_array[i].type, '\0', 12);
		temp_array[i].num_connections = 0;
		for (int j=0; j < NUM_CONNECTIONS; j++){
			temp_array[i].outbound_connections[j] = NULL;
		}
	}
	return temp_array;
}

/******************************************************************************
 *	   Description: Searches array of rooms for room with given string
 *	    Parameters: array of struct room, and room name as a c-string
 *	       Returns: room with desired name
 *	Pre-conditions: room_name exists in the rooms_array
 *     Post-conditions: returns a struct room
******************************************************************************/
struct 
room* get_room(struct room* rooms_array, char* room_name) {
	for(int i = 0; i < NUM_ROOMS; i++){
		if(strcmp(rooms_array[i].name, room_name) == 0) {
		    return &(rooms_array[i]);
		}
	}
	return NULL;
}

/******************************************************************************
 *	   Description: Searches array of rooms for the room with type START_ROOM
 *	    Parameters: receives the array of rooms
 *	       Returns: struct room
 *	Pre-conditions: rooms_array is not empty
 *     Post-conditions: room struct is returned
******************************************************************************/
struct 
room* get_start(struct room* rooms_array){
	for(int i = 0; i < NUM_ROOMS; i++){
	    	if(strcmp(rooms_array[i].type, "START_ROOM") == 0 ) {
			return &(rooms_array[i]);
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
	strcpy(filename, FILE_NAME);
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
 *	   Description: reads a line from file that holds the current time
 *	    Parameters: none
 *	       Returns: none
 *	Pre-conditions: file exists and is readable
 *     Post-conditions: current time printed to screen
******************************************************************************/
void 
read_time_file(){
	FILE *fptr;
	char time_string[64];
	memset(time_string, '\0', 64);
	char filename[16];
	memset(filename, '\0', 16);
	strcpy(filename, FILE_NAME);

    // keep trying until file can be opened
	while (!(fptr = fopen(filename, "r")))
	    ;
    fgets(time_string, 64, fptr);
	clear_screen();
	printf("%s\n", time_string);
	fclose(fptr);
}
/******************************************************************************
 *	    Description: checks user input to match a room name in the current
 *	   		          room's outboundConnections
 *	     Parameters: Receives user input and address of current room
 *	        Returns: if user_input doesn't match a outbound room name return -1
 *	        		    otherwise returns index of room
 *	 Pre-conditions: user_input is not empty, current_room is not empty
 *  Post-conditions: int is returned
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
 *	   Description: gets path to room file
 *	    Parameters: none
 *	       Returns: array of strings (char*)
 *	Pre-conditions: files exist
 *     Post-conditions: 2d array is returned to calling function
******************************************************************************/
char** 
get_file_paths(){
	char** filepath_array;
	filepath_array = calloc(NUM_ROOMS, sizeof(char*));

    	for (int  i = 0; i < NUM_ROOMS; i++){
		filepath_array[i] = calloc(64, sizeof(char));
		memset(filepath_array[i], '\0', 64);
	}

	char file_suffix[6] = "_Room";
	char* my_dir = read_my_dir();
	DIR* dir_ptr = opendir(my_dir);

	if (dir_ptr > 0) {
		int index = 0;
		struct dirent* file_in_dir;
		while ((file_in_dir = readdir(dir_ptr))) {
			if (strstr(file_in_dir->d_name, file_suffix)) {
				sprintf(filepath_array[index], "%s/%s", my_dir, file_in_dir->d_name);
				index++;
			}
		}
		closedir(dir_ptr);
		free(my_dir);
		return filepath_array;
	}
	closedir(dir_ptr);
	free(my_dir);

	// for compiler
	return NULL;
}

/******************************************************************************
 *	   Description: searches for directory with onid username
 *	    Parameters: none
 *	       Returns: receives array of char holding directory name
 *	Pre-conditions: directory exists
 *     Post-conditions: returns string containing newest directory
******************************************************************************/
char* 
read_my_dir(){
    	int newest_dir_time = -1;
    	char dir_prefix[17] = "adventure.rooms.";
    	char* newest_dir_name;
    	newest_dir_name = (char*) malloc(33 * sizeof(char));
    	memset(newest_dir_name, '\0', 33);

    	DIR* dir_ptr;
    	struct dirent* file_in_dir;
    	struct stat dir_attributes;

    	dir_ptr = opendir(".");
    	
        if (dir_ptr > 0 ) {
            // iterate over files in current directory
    		while ((file_in_dir = readdir(dir_ptr))) {
			    // check if current file begins with dir_prefix which contains
                //  room files
                if (strstr(file_in_dir->d_name, dir_prefix)) {
		    	    // get file attributes of directory as a struct stat
                    stat(file_in_dir->d_name, &dir_attributes);
		    		
                    // compare modified time of file to current newest time
                    if( (int)dir_attributes.st_mtime > newest_dir_time){
				        newest_dir_time = (int)dir_attributes.st_mtime;
				        memset(newest_dir_name, '\0', 33);
				        strcpy(newest_dir_name, file_in_dir->d_name);
		    		}//end of if file newer than previous newest file
			    }//end of if filename contains my str
    		}//end while loop

		    if ( newest_dir_name[0] == '\0') {
			    printf("Can not find correct directory!\n");
			    free(newest_dir_name);
			    exit(1);
		    }
    	}//end if dirPtr
    	closedir(dir_ptr);

	return newest_dir_name;
}

/******************************************************************************
 *	   Description: Prints information for current room
 *	    Parameters: receives address to current room
 *	       Returns: none
 *	Pre-conditions: current_room is not NULL
 *     Post-conditions: room information printed to screen
******************************************************************************/
void 
print_room_info(struct room* current_room){
	int i = 0;
	
    printf("CURRENT LOCATION: %s\nPOSSIBLE CONNECTIONS:", current_room->name);
	
    for (i=0; i < current_room->num_connections; i++){
		//if (i < (current_room->num_connections-1) ) {
	    		printf(" %s ", current_room->outbound_connections[i]->name);
		//}
		//else if (i == (current_room->num_connections-1)) {
	    //		printf(" %s.\n", current_room->outbound_connections[i]->name);
		//}
	}
    printf("\n");
}


/******************************************************************************
 *	   Description:	Attaches the connections to each room struct
 *	    Parameters: rooms array and 2d array holding filepaths
 *	       Returns: none
 *	Pre-conditions: rooms_array is filled with name and type for each room
 *     Post-conditions: outboundConnections array for each room contains the
 *     			correct address for connected rooms
******************************************************************************/
void 
attach_connections(struct room* rooms_array, char** filepath_array) {
	FILE* fptr;

	for(int i = 0; i < NUM_ROOMS; i++){
	    int connection_index = 0;
		char arg1[12], arg2[12], arg3[12];
		memset(arg1, '\0', 12); memset(arg2, '\0', 12); memset(arg3, '\0', 12);
	    
        fptr = fopen(filepath_array[i], "r");
        if (!fptr) {
		    perror("Error reading file.");
		    exit(1);
		}

		while((fscanf(fptr, "%s %s %s", arg1, arg2, arg3)) != EOF){
			if (!strcmp(arg1, "CONNECTION")) {
				struct room* connection = get_room(rooms_array, arg3);
				rooms_array[i].outbound_connections[connection_index] = connection;
				rooms_array[i].num_connections++;
				connection_index++;
			}
		    	memset(arg1, '\0', 12); memset(arg2, '\0', 12); memset(arg3, '\0', 12);
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
read_files(struct room* rooms_array, char** filepath_array){
	FILE* fptr;

	for(int i = 0; i < NUM_ROOMS; i++){
	    char arg1[12], arg2[12], arg3[12];
		memset(arg1, '\0', 12);
		memset(arg2, '\0', 12);
		memset(arg3, '\0', 12);

		fptr = fopen(filepath_array[i], "r");

		if(!fptr) {
		    perror("Error reading file.");
		    exit(1);
		}

		// read through room file
		while ((fscanf(fptr, "%s %s %s", arg1, arg2, arg3)) != EOF){
			if (!strcmp(arg1, "ROOM")) {
                if (!strcmp(arg2, "TYPE:")) {
                    strcpy(rooms_array[i].type, arg3);
                }
                else {
                    strcpy(rooms_array[i].name, arg3);
                }
			}
		    	memset(arg1, '\0', 12); memset(arg2, '\0', 12); memset(arg3, '\0', 12);
        }
        assert(rooms_array[i].name && rooms_array[i].type);

		fclose(fptr);
	}

	attach_connections(rooms_array, filepath_array);
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
	if(filepaths_array != NULL){
		for(int i = (NUM_ROOMS-1); i >= 0; i--) {
			free(filepaths_array[i]);
		}
		free(filepaths_array);
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
	if (rooms_array != NULL) {
    		for(int i = 0; i < NUM_ROOMS; i++){
			free(rooms_array[i].name);
	 		free(rooms_array[i].type);
		}
		free(rooms_array);
	}
}

#endif
