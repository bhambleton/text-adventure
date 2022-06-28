#ifndef _BUILDROOMS_C_
#define _BUILDROOMS_C_

#include "buildrooms.h"

int 
main(void) {
    struct room* rooms_array = NULL;
        
    srand(time(NULL));
    rooms_array = allocate_rooms();	
    build_files(rooms_array);	
    deallocate(rooms_array);

    return 0;
}


/******************************************************************************
 *	   Description: prints contents of a struct to a file
 *	    Parameters: receives file pointer and room struct
 *	       Returns: none
 *	Pre-conditions: fptr points to valid file stream
 *     Post-conditions: contents of struct written to file
******************************************************************************/
void 
write_file_info(FILE* fptr, struct room room) {
    fprintf(fptr, "ROOM NAME: %s\n", room.name);
    for(int i = 0; i < room.num_connections; i++){
       fprintf(fptr, "CONNECTION %d: %s\n", 
           (i+1), 
           room.outbound_connections[i]->name);
    }
        fprintf(fptr, "ROOM TYPE: %s\n", room.type);
}

/******************************************************************************
 *	   Description: builds directory and its files
 *	    Parameters: receives array of room structs
 *	       Returns: none
 *	Pre-conditions: rooms_array is not null and contents initialized
 *     Post-conditions: directory and 7 files are created and 
 *     			filled with contents of rooms_array
******************************************************************************/
void 
build_files(struct room* rooms_array) {
    char dir_name[32];
    int pid = getpid();

    /*combine PID & username into one string*/
    sprintf(dir_name, "adventure.rooms.%d", pid);
    mkdir(dir_name, 0755); //create directory
    
    for(int i = 0; i < NUM_ROOMS; i++) {
        FILE* fptr;
        char path_name[64];
        memset(path_name, '\0', 64);

        //create a string containing path to file to create 
        sprintf(path_name, "%s/%s", dir_name, rooms_array[i].name);

        //create file
        fptr = fopen(path_name, "w");
        if (fptr == NULL) { 
            printf("Could not open file: %s", path_name); exit(1); 
        }

        write_file_info(fptr, rooms_array[i]);
        fclose(fptr);		
    } //end loop
}

/******************************************************************************
 *	   Description: generates the array of rooms
 *	    Parameters: none
 *	       Returns: pointer to array of struct room
 *	Pre-conditions: has required amount of mem available
 *     Post-conditions: memory allocated
******************************************************************************/
struct 
room* allocate_rooms() {
    struct room* rooms_array = (struct room*) malloc(NUM_ROOMS *sizeof(struct room));
    
    for (int i = 0; i < NUM_ROOMS; i++) {
        rooms_array[i].name = (char*) calloc(16, sizeof(char));
        memset(rooms_array[i].name, '\0', 16);
        rooms_array[i].type = (char*) calloc(12, sizeof(char));
        memset(rooms_array[i].type, '\0', 12);
        rooms_array[i].num_connections = 0;
        
        for(int j = 0; j < NUM_CONNECTIONS; j++){
            rooms_array[i].outbound_connections[j] = NULL;
        }
    }

    generate_room_names_types(rooms_array);
    
    while (is_graph_full(rooms_array) == 0) {
        add_random_connection(rooms_array);
        }
    
    return rooms_array;
}

/******************************************************************************
 *	   Description: frees memory used for array of struct room
 *	    Parameters: receives array of struct room
 *	       Returns: none
 *	Pre-conditions: rooms_array contains array of struct room type
 *     Post-conditions: rooms_array is NULL
******************************************************************************/
void 
deallocate(struct room* rooms_array) {

    if (rooms_array != NULL) {
        for (int i = 0; i < NUM_ROOMS; i++) {
            free(rooms_array[i].name);
            free(rooms_array[i].type);
        }
        free(rooms_array); 
    }
    rooms_array = NULL;
}

/******************************************************************************
 *	   Description: sets random names from hardcoded array of names
 *	    Parameters: receives array of room structs
 *	       Returns: none 
 *	Pre-conditions: rooms array is initialized
 *     Post-conditions: each index in rooms array has name
******************************************************************************/
void 
generate_room_names_types(struct room* rooms_array) {

    for (int i = (NUM_ROOMS-1); i >= 0; i--) {
        //pick random name from array
        int index = rand() % 10;
        if ((i == NUM_ROOMS - 1) && (ROOM_NAMES[index] != NULL)) {
                strcpy(rooms_array[i].name, ROOM_NAMES[index]); 
                strcpy(rooms_array[i].type, "END_ROOM"); 
        }
        else {
            int iter = (NUM_ROOMS - 1);
            //loop to find unused room name
            while ( iter >= i ) {
                //check if name was already stored in temp_list
                if (!strcmp(ROOM_NAMES[index], rooms_array[iter].name)){ 
                    //if random index is already in temp_list
                    //index gets new number & break out of while loop 
                    index = rand() % 10;
                    iter=NUM_ROOMS;
                }
                iter--; //continues until it does not find a match
            }
            
            strcpy(rooms_array[i].name, ROOM_NAMES[index]);

            if ( i == 0 ) { strcpy(rooms_array[i].type, "START_ROOM"); }
            else { strcpy(rooms_array[i].type, "MID_ROOM"); }
        }

    } //end loop to fill temp_list with room names
}

/******************************************************************************
 *	   Description: checks if each room has between
 *		 	3 & 6 connections
 *	    Parameters: receives array of room structs
 *	       Returns: returns 1 if full, 0 if not full
 *	Pre-conditions: array exists and is not NULL
 *     Post-conditions: returns an integer
******************************************************************************/
int 
is_graph_full(struct room rooms_array[NUM_ROOMS]) {
    int ready_check = 0, c_count = 0;
    
    for (int i = 0; i < NUM_ROOMS; i++) {
        c_count = rooms_array[i].num_connections;
        if ((c_count >= 3) && (c_count <= NUM_CONNECTIONS)) { ready_check++; }
    }

    if (ready_check==NUM_ROOMS) { return 1; }
    else { return 0; }

}

/******************************************************************************
 *	   Description: Gets two unconnected rooms and connects them 
 *	    Parameters: receives array of struct room
 *	       Returns: none
 *	Pre-conditions: rooms_array is not NULL
 *     Post-conditions: two rooms in the array are connected
******************************************************************************/
void 
add_random_connection(struct room rooms_array[NUM_ROOMS]) {
    struct room* A = NULL;
    struct room* B = NULL;

    do{
        A = get_random_room(rooms_array);
    } while(!can_add_connection_from((*A))); 


    do{ 
            B = get_random_room(rooms_array);
    } while(!can_add_connection_from((*B)) || is_same_room((*A),(*B)) || connection_already_exists((*A),(*B)));

    connect_room(A,B);
    connect_room(B,A);
}

/******************************************************************************
 *	   Description: gets random index from array
 *	    Parameters: receives array of struct room type
 *	       Returns: address of array index
 *	Pre-conditions: rooms_array is not NULL
 *     Post-conditions: address to a random index is returned
******************************************************************************/
struct 
room* get_random_room(struct room rooms_array[NUM_ROOMS]) {
    return &rooms_array[rand() % NUM_ROOMS];
}

/******************************************************************************
 *	   Description: Boolean function to see if room has less than 6 connections
 *	    Parameters: receives room struct
 *	       Returns: 1 if true, 0 if false
 *	Pre-conditions: x contains room struct
 *     Post-conditions: returns an integer
******************************************************************************/
int 
can_add_connection_from(struct room x) {
    return (x.num_connections < NUM_CONNECTIONS) ? 1 : 0;
}

/******************************************************************************
 *	   Description: checks if a connection to y exists in x
 *	    Parameters: receives two struct room types
 *	       Returns: 1 if true, 0 if false
 *	Pre-conditions: x and y contain a struct room
 *     Post-conditions: integer is returned
******************************************************************************/
int 
connection_already_exists(struct room x, struct room y) {
    int iter = x.num_connections;
    if (iter == 0) return 0;
    
    for (int index = 0; index < NUM_CONNECTIONS; index++) {
        if (x.outbound_connections[index] == NULL) continue;
            if (!strcmp(x.outbound_connections[index]->name, y.name)) return 1;
    }
    
    return 0;
}

/******************************************************************************
 *	   Description: checks if the two structs have same name
 *	    Parameters: receives two structs type room
 *	       Returns: 1 if they are the same, 0 if not
 *	Pre-conditions: x and y represent a struct room
 *     Post-conditions: returns integer
******************************************************************************/
int 
is_same_room (struct room x, struct room y) {
    return (strcmp(x.name, y.name)) ? 0 : 1;
}

/******************************************************************************
 *	   Description: adds room y to outboundC array in x
 *	    Parameters: receives two struct room addresses 
 *	       Returns: none
 *	Pre-conditions: x and y hold address to struct room
 *     Post-conditions: address of y in outboundC array of x
******************************************************************************/
void 
connect_room(struct room* x, struct room* y) {
    x->outbound_connections[x->num_connections] = y;
    x->num_connections += 1;
}
#endif
