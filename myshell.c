#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

//function declarations
char* readline();
char** parse(char* input);
int execute(char** args);
//builtin functions
int quit(char** args);
int cd(char* directory);
int clear(char** args);
int dir(char* directory);
int environ(char** args);
int echo(char* comment);
int help(char** args);

#define BUFSIZE  1024

//list of builtin command keywords
char* builtins[] = {"quit", "cd", "clear", "dir", "environ", "echo", "help"};
int (*builtinFN[])(char **) = {&quit, &cd, &clear, &dir, &environ, &echo, &help};

// The main function: infinite loop that is the shell
void main(int argc, char*argv[]) {
  int a = 1;
  while(a == 1) {
    printf("myshell>");
    char* input = readline();
    //this printf is just for testing purposes
    //printf("%s\n", input);
    char** inputArray = parse(input);
    a = execute(inputArray);
  }
}

//read user input until user hits enter, then return that input string
char* readline() {
  char* input = malloc(sizeof(char) * BUFSIZE);
  int pos = 0;
  char c;
  while((c = getchar()) != '\n' & c != EOF) {
    input[pos] = c;
    pos++;
  }
  input[pos] = '\0';
  return input;
  //TODO: handle if input is longer than BUFSIZE, need to allocate more space to input
}

//split user input into an array of char* based on spaces
char** parse(char* input) {
  int i = 0;
  char** arrayArgs = malloc(sizeof(char) * BUFSIZE);
  const char* space = " ";
  char* arg = strtok(input, space);

  while(arg != NULL) {
    arrayArgs[i] = arg;
    //this printf just for testing purposes (to confirm args were split correctly)
    //printf("args %d = %s\n", i, arrayArgs[i]);
    i++;
    arg = strtok(NULL, space);
  }
  arrayArgs[i] = NULL;
  return arrayArgs;
}

//based on values of args array, execute appropriate function (either builtin or external)
int execute(char** args) {
  int i = 0;
  int fn;
  pid_t pid;

  pid = fork();
  if(pid < 0) {
    printf("Error forking\n");
  }
  else if(pid == 0) {
    //in child
    //TODO: check if we need to redirect I/0 or run in background

    //check for builtin, then run matching fn if it is
    int j;
    for(j = 0; j < 7; j++) { //j = number of builtin functions there are
      if(strcmp(args[0], builtins[j]) == 0) {
        return(builtinFN[j])(args);
      }
    }
    //input wasn't a builtin
    printf("Command or executable not recognized.\n");
    return 1;
    //TODO: exec the external executable file if it exists
  }
  else {
    //in parent
    waitpid(pid, NULL, 0);
  }
}

//---------------------builtin functions------------------------------

//quit shell
int quit(char** args) {
  return 0;
}

int cd(char* directory) {
  if(chdir(directory) != 0) {
    printf("No directory '%s' found.\n", directory);
  }
  return 1;
}

int clear(char** args) {
  int screenHeight = 50;
  int i;
  for(i = 0; i < screenHeight; i++) {
    printf("\n");
  }
  return 1;
}

int dir(char* directory) {
  printf("dir command executed\n");
  return 1;
}

int environ(char** args) {
  printf("environ\n");
  return 1;
}

int echo(char* comment) {
  printf("%s\n", comment);
  return 1;
}

//print out explanation of how to use the shell
int help(char** args) {
  printf("myShell HELP\n");
  return 1;
}

