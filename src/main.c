#include "main.h"
#define MAX_TOKEN 100

void print_error_message(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    fflush(NULL);
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

    char *token; 
    char *token_array[MAX_TOKEN]; //array contains tokens from user input
    char *delim = " \t\n"; //delimit set for strsep
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
        token_array[token_count] = NULL;  //make sure last item in the token array a NULL terminator
        
        // built-in commands check
        if (strcmp(token_array[0], "exit") == 0) { //exit command
            if (token_count > 1){ //checking if there is more than one argument for exit command
                print_error_message();
                continue;
            }
            free(lineptr);
            exit(0);
        } else if (strcmp(token_array[0], "cd") == 0) { // cd command
            if (token_count != 2) { // cd can only take one argument
                print_error_message();
                continue;
            } else {
                if (chdir(token_array[1]) != 0) {
                    print_error_message();
                }
            }
            continue;
        } else if (strcmp(token_array[0], "path") == 0) { //path command
            //free old allocated strings
            for (int i = 0; i < path_count; i++){
                free(search_path[i]);
            }
            path_count = 0; // reset path count to 0

            // add new paths from user's input
            for (int i = 1; i < token_count ; i++){
                search_path[path_count] = strdup(token_array[i]); //copy tokens from token array to search path
                path_count++;
            }
            continue;
        }
        
        // 
        char full_path[256];
        int found = 0;
        for (int i = 0; i < path_count ; i++){
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

        //checking for redirection mark ">"
        int rdr_idx = -1;
        char *filename = NULL;
        int error = 0; 
        for (int i = 0; i < token_count; i++){
            if (strcmp(token_array[i], ">") == 0){
                if (rdr_idx != -1) { // checking for extra ">"
                    print_error_message();
                    error = 1; // flag 
                }
                if (i+1 >= token_count || i+2 != token_count) { // detect no filename given or extra arguments after filename
                    print_error_message(); 
                    error = 1;
                    break;
                }
                rdr_idx = i; // remember index of redirection 
                filename = token_array[i+1];
                token_array[rdr_idx] = NULL; // cut off > and filename from token array so execv won't pass them as arguments
                break; // only support one ">"
            }
        }

        if( error ) { // if there is error flag , skip fork() and execv(), go back to prompt
            continue;
        }
        
        //using fork(), create a child process
        int rc = fork();
        if (rc < 0) {
            //forked failed
            print_error_message();
            exit(1);
        } else if (rc == 0) {
            //child, run a new process
            if (rdr_idx != -1){ // checking for redirection
                int file_descriptor = open(filename, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
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
            //parent , wait for child to finish
            wait(NULL);
        }
    }
    free(lineptr);
    return(0);
}

