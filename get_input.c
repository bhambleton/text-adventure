#ifndef _GET_INPUT_C
#define _GET_INPUT_C

#include "get_input.h"

// Driver
/*
int main(int argc, char* argv[]){
	struct user_input* input = (struct user_input*) malloc(sizeof(struct user_input));
	memset(input, '\0', sizeof(struct user_input));
	input->input = NULL;
	input->length = 0;

	if(!get_user_input(input)) {
		printf("Input:\t'%s'\nLength:\t%d\n", input->input, input->length);
		free(input->input);
		input->input = NULL;
	}

	if (input->input != NULL)
		free(input->input);

	free(input);
	char* u_input = NULL;
	int length = 0, temp = 0;
    printf("< ");
	temp = get_line(&u_input, &length, stdin);
	printf("Input:\t'%s'\nBuffer Length:\t%d\nInput Length:\t%d\n", u_input, length, temp);
	free(u_input);
	return 0;
}
*/

int alloc_line(char** buffer, int* buffer_length, int new_length) {
    	char* new_buffer = realloc((*buffer), new_length);
    	if (!new_buffer)
		return -1;

	(*buffer) = new_buffer;
	(*buffer_length) = new_length;

	return 0;
}

int get_line (char** buffer, int* buffer_length, FILE* fptr) {
	if ((*buffer_length) <= 0) {
		if (alloc_line(buffer, buffer_length, 2) == -1) {
			return -1;
		}
	}

	if (!(*buffer)) {
		return -1;
	}

	char* buffer_ptr = (*buffer);
	int new_length = (*buffer_length);

	while (1) {
		//char* read_chars = fgets(buffer_ptr, new_length, fptr);

		if (fgets(buffer_ptr, new_length, fptr) == NULL)
			return -1;

        const size_t read_size = strlen(*buffer);
		// check if end of file or end of input is newline char
		if (feof(fptr) || *((*buffer) + read_size - 1) == '\n') {
		    	// remove '\n' char from input
			    *((*buffer) + read_size - 1) = '\0';
            // return full input string
			return ((*buffer) + read_size - 1) - (*buffer);
		}

		new_length = (*buffer_length) + 1;

		if (alloc_line(buffer, buffer_length, (*buffer_length) * 2))
			return -1;

		// adjust pointer to the end of the input chars currently
		// 	stored in buffer
		buffer_ptr = (*buffer) + (*buffer_length) - new_length;
	}
}

int get_user_input(struct user_input* user_input){
	int bufferSize = 0;
	char* user_raw_input = NULL;

	while (1){
		printf("> ");
		user_input->length = get_line(&user_raw_input, &bufferSize, stdin);
		if (user_input->length == -1) {
			perror("Error getting input: ");
			return 1;
		}
		else { break; }
	}

	// Copy input from get_line into input struct
	// 	+ trailing null char (not included in userInput->length)
	user_input->input = (char*) malloc(1 + user_input->length * sizeof(char));
	if (user_input->input == NULL) {
		perror("Error allocating input buffer: ");
		return 1;
	}
	memset(user_input->input, '\0', user_input->length + 1);
	memcpy(user_input->input, user_raw_input, user_input->length);

	free(user_raw_input);

	return 0;
}
#endif
