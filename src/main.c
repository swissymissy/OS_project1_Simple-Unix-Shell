/*
 * rush - A simplified Unix shell implementation
 *
 * Description:
 * This program implements a basic shell that supports interactive and batch modes.
 * It handles built-in commands (exit, cd, path), executes external programs,
 * supports output redirection using '>' and parallel command execution using '&'.
 * Executables are searched using user-defined paths, validated via access().
 *
 * Name: Phuong Nghi Hoang
 * Student ID: 8765-3734
 * Date: Sep 17th 2025
 */

#include "main.h"
#define MAX_TOKEN 100

void print_error_message(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    fflush(NULL);
}

void split_commands(char *command, char **command_array) {
    int i = 0;
    char *token;
    while ((token = strsep(&command, " \t\n")) != NULL) {
        if (*token == '\0') {
            continue;
        }
        command_array[i] = token;
        i++; 
    }
    command_array[i] = NULL; // making sure last value is NULL terminator
}

int main(int argc, char *argv[]){
    if (argc > 2) {
        print_error_message();
        exit(1);
    }
    FILE *input_stream = stdin; // default: interactive mode

    if (argc == 2) { //user enter an argument to read command from an input
        input_stream = fopen(argv[1], "r");
        if (input_stream == NULL){
            print_error_message();
            exit(1);
        }
    }
    char *prompt = "rush> ";
    char *lineptr = NULL;
    size_t n = 0;
    ssize_t input;

    char *search_path[MAX_TOKEN];
    search_path[0] = strdup("/bin/"); //heap-allocated string, default path for not built-in commands
    search_path[1] = strdup("/usr/bin");
    int path_count = 2;

    //create infite loop:
    while(1){
        if (argc == 1) {
            printf("%s", prompt);
            fflush(stdout);
        }
        input = getline(&lineptr, &n, input_stream); //read the inputs 
        //handle EOF of getline
        if (input == -1){
            return(-1);
        }
        
        // remove trailing newline
        lineptr[strcspn(lineptr, "\n")] = '\0';
        if (*lineptr == '\0') {
            continue;
        }

        char *linecopy = lineptr;
        char *commands[MAX_TOKEN];
        char *cmd;
        int num_cmd = 0;

        //splitting full command by "&"
        while((cmd = strsep(&linecopy, "&")) != NULL) {
            // trim leading spaces
            while (*cmd == ' ' || *cmd == '\t'){
                cmd++;
            }
            if (*cmd == '\0'){
                continue;
            }
            commands[num_cmd]= strdup(cmd); // storing command to the command array
            num_cmd++;
        }
       
        if (num_cmd == 0) {
            continue; //empty line, prompt again
        }
        
        pid_t processID[MAX_TOKEN]; // array to store the children processes for parallel commands
    
        // process each command in commands array 
        for (int i = 0; i < num_cmd; i++){
            char *token_array[MAX_TOKEN]; // array contains tokens from splitting
            split_commands(commands[i], token_array); // split command into tokens

            // built-in command check
            if (strcmp(token_array[0], "exit") == 0) { //exit command
                if (token_array[1] != NULL){ // exit command can't take any argument
                    print_error_message();
                    continue;
                }
                free(lineptr);
                exit(0);
            } else if (strcmp(token_array[0], "cd") == 0) { // cd command
                if (!token_array[1] || token_array[2]) { // cd can take only one argument
                    print_error_message();
                    continue;
                } else {
                    if (chdir(token_array[1]) != 0) {
                        print_error_message();
                    }
                }
                continue;
            } else if (strcmp(token_array[0], "path") == 0) { //path command
                // if no argument, clear path list
                if(token_array[1] == NULL) {
                    for (int i = 0; i < path_count; i++){
                        free(search_path[i]);
                    }
                    path_count = 0;
                    continue;
                }
                // validate if the path exists and is a directory
                int error = 0;
                for (int i = 1; token_array[i] != NULL; i++) {
                    if (access(token_array[i], F_OK | X_OK) != 0){ 
                        print_error_message(); 
                        error = 1;
                        break;
                    }
                }
                if(error) {
                    continue; //skip updating path list
                }
                //free old allocated strings
                for (int i = 0; i < path_count; i++){
                    free(search_path[i]);
                }
                path_count = 0; // reset path count to 0

                // add new paths from user's input
                for (int i = 1; token_array[i] != NULL ; i++){
                    search_path[path_count] = strdup(token_array[i]); //copy tokens from token array to search path
                    path_count++;
                }
                continue;
            }
        
            // search for executables
            char full_path[256];
            int found = 0;
            for (int i = 0; i < path_count ; i++){
                if (search_path[i][strlen(search_path[i]) - 1] == '/'){ // checking if the path ends in '/' and adding it if needed
                    snprintf(full_path, sizeof(full_path), "%s%s", search_path[i], token_array[0]);
                } else {
                    snprintf(full_path, sizeof(full_path), "%s/%s", search_path[i], token_array[0]);
                }

                if (access(full_path, X_OK) == 0) { //checking if it is executable
                    found = 1;
                    break;
                }
            }
            if (!found) {
                print_error_message();
                continue; //back to prompt
            }

            //checking for redirection mark ">"
            int rdr_idx = -1;
            char *filename = NULL;
            int error = 0; //flag
            for (int i = 0; token_array[i] != NULL; i++){
                if (strcmp(token_array[i], ">") == 0){
                    if (rdr_idx != -1) { // checking for extra ">"
                        print_error_message();
                        error = 1; // flag 
                    }
                    if (token_array[i+1] == NULL  || token_array[i+2] != NULL) { // detect no filename given or extra arguments after filename
                        print_error_message(); 
                        error = 1;
                        break;
                    }
                    rdr_idx = i; // remember index of redirection 
                    filename = token_array[i+1];
                    token_array[rdr_idx] = NULL; // cut off > so execv won't pass them as arguments
                    token_array[rdr_idx+1] = NULL; // ensure filename doesn't get passed to execv() as argument
                    break; // only support one ">"
                }
            }

            if( error ) { // if there is error, skip fork() and execv(), go back to prompt
                continue;
            }
        
            // create child process
            int rc = fork();
            if (rc < 0) {
                //forked failed
                print_error_message();
                exit(1);
            } else if (rc == 0) {
                //child, run a new process
                if (rdr_idx != -1){ // checking for redirection
                    int file_descriptor = open(filename, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU); // save the descriptor of the opened file
                    if (file_descriptor < 0){
                        print_error_message();
                        exit(1);
                    }
                    dup2(file_descriptor, STDOUT_FILENO); // redirect output stream to the new opened file
                    close(file_descriptor); // close this descriptor, no need anymore
                }
                token_array[0] = full_path;
                execv(full_path, token_array);
                print_error_message(); // if execv fails. print error message
                exit(1);
            } else {
                //save each child's process ID to the process array
                processID[i] = rc;
            }
            free(commands[i]);
        }
        
        //wait for all children process to finish
        for (int i = 0; i < num_cmd; i++){
            waitpid(processID[i], NULL, 0);
        }
    }
    free(lineptr);
    return(0);
}

