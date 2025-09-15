#include "main.h"
#define MAX_TOKEN 100

void print_error_message(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    fflush(NULL);
}
int main(void){
    char *prompt = "rush> ";
    char *lineptr = NULL;
    size_t n = 0;
    ssize_t input;

    char *token; 
    char *token_array[MAX_TOKEN]; //array contains tokens from user input
    char *delim = " \t\n";
    const char *built_in_cmds[] = {"exit", "cd", "path", NULL}; // array of built-in commands

    //create infite loop:
    while(1){
        printf("%s", prompt);
        fflush(stdout);
        input = getline(&lineptr, &n, stdin); //read the inputs 
        //handle EOF of getline
        if (input == -1){
            return(-1);
        }
        
        //splitting the full command line into tokens
        char *linecopy = lineptr;
        int token_count = 0;
        while ((token = strsep(&linecopy, delim)) != NULL) {
            if (*token == '\0'){
                continue; //skip empty tokens
            }
            token_array[token_count] = token; //storing current token to the token array
            token_count++;
        }
        if (token_count == 0) {
            continue; //empty line, prompt again
        }
        token_array[token_count] = NULL; 
        
        // built-in commands check
        for (int i = 0; built_in_cmds[i] != NULL ; i++ ) {
            if (strcmp(token_array[0], built_in_cmds[i]) == 0) {
                if (i == 0) { //exit command
                    if (token_count > 1){ //checking if there is more than one paramter for exit command
                        print_error_message();
                        continue;
                    }
                    free(lineptr);
                    exit(0);
                } else if (i == 1) { // cd command
                    if (token_count != 2) {
                        print_error_message();
                        continue;
                    } else {
                        if (chdir(token_array[1]) != 0) {
                            print_error_message();
                        }
                    }
                    continue;
                } else if (i == 2) {
                    
                }
            }
        }

        char *search_path[] = {"/bin/", "/usr/bin/", NULL};
        char full_path[256];
        int found = 0;
        for (int i = 0; search_path[i] != NULL ; i++){
            snprintf(full_path, sizeof(full_path), "%s%s", search_path[i], token_array[0]);
            if (access(full_path, X_OK) == 0) { //checking if it is executable
                found = 1;
                break;
            }
        }
        if (!found) {
            print_error_message();
            continue; //back to prompt
        }
        //using fork(), create a child process
        int rc = fork();
        if (rc < 0) {
            //forked failed
            print_error_message();
            exit(1);
        } else if (rc == 0) {
            //child, run a new process
            execv(full_path, token_array);
            print_error_message(); // if execv fails. print error message
            exit(1);
        } else {
            //parent , wait for child to finish
            wait(NULL);
        }
    }
    free(lineptr);
    return(0);
}

