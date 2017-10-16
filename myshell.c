#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>

//function declarations
char* readline();
char** parse(char* input);
int execute(char** args);
//builtin functions
int quit(char** args);
int cd(char** args);
int clear(char** args);
int dir(char** args);
int environ(char** args);
int echo(char** args);
int help(char** args);

#define BUFSIZE  1024

//list of builtin command keywords
char* builtins[] = {"quit", "cd", "clear", "dir", "environ", "echo", "help"};
int (*builtinFN[])(char **) = {&quit, &cd, &clear, &dir, &environ, &echo, &help};

// The main function: loop that is the shell prompt
void main(int argc, char*argv[]) {
  //clear screen when entering shell
  system("clear");
  //a will be the exit code/return value of any executed program or function
  int a = 1;
  while(a != 0) {
    //get current directory to print in prompt
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    //print prompt
    printf("$myShell:~%s/myshell>", cwd);
    char* input = readline();
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
//returns 0 if 'quit' was called, returns nonzero otherwise
int execute(char** args) {
  int ret = 1; //returns 0 on quit, 1 otherwise

  int isBuiltin = 0;
  int isRedirect = 0;
  int inBg = 0;
 
  int exitStat, i, j;

  //check if we need to run in background   
  i = 0;
  while(args[i] != NULL) {
    if(strcmp(args[i], "&") == 0) {
      inBg = 1;
    } 
 /*  if(strcmp(args[i], "<") == 0) {
      printf("redirect input to %s\n", args[i+1]);
    }
    else if(strcmp(args[i], ">") == 0) {
      printf("redirect output to %s\n", args[i+1]);
    } 
    else if(strcmp(args[i], ">>") == 0) {
      printf("redirect output to %s\n", args[i+1]);
    } */
    i++;
  } 

  //check for builtin, then run matching fn if it is
  for(j = 0; j < 7; j++) { //j = number of builtin functions there are
    //check if first arg matches a builtin
    if(strcmp(args[0], builtins[j]) == 0) {
      //check if output needs redirecting
      i = 0;
      while(args[i] != NULL) {
        if(strcmp(args[i], ">") == 0) {
          //dupe then dup2
          isRedirect = 1;
        }
        else if(strcmp(args[i], ">>") == 0) {
          //dup then dup2
          isRedirect = 1;
        }
        i++;
      }    
      //run function and set return value
      ret = (builtinFN[j])(args);
      //reset STDOUT if we changed it
      if(isRedirect == 1) {

      }
      isBuiltin = 1;
    }
  }

  //if we didn't find a builtin fn, fork and try to exec a file 
  if(isBuiltin == 0) {
    //create a pipe
    int fd[2];
    if(pipe(fd) == -1) {
      printf("Pipe error\n");
    }

    //FORK to create child
    pid_t pid;
    pid = fork();
    if(pid < 0) {
      printf("Error forking\n");
    }
    else if(pid == 0) {
      //in child
      execvp(args[0], args) || 1;
      //there was no file to exec, so tell the user they're wrong
      printf("Command or executable not recognized.\n");
      //exit with return value
      exit(ret);
    }
    else {
      //in parent
      //wait for child to finish if we aren't running in bg
      if(inBg == 0) {
        waitpid(pid, &exitStat, 0);
      }
      return exitStat;
    }
  }
  return ret;
}

//---------------------builtin functions------------------------------

//quit shell
int quit(char** args) {
  return 0;
}

int cd(char** args) {
  if(chdir(args[1]) != 0) {
    printf("No directory '%s' found.\n", args[1]);
  }
  return 1;
}

int clear(char** args) {
  //system("clear"); //this function isn't portable, only works in UNIX

  //get height of window so we know how many lines to print at a time
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  int r;
  for(r = 0; r < w.ws_row; r++) {
    printf("\n");
  }
  return 1;
}

int dir(char** args) {
  DIR *dir;
  struct dirent *dirContents;
  if(args[1] == NULL) {
    //if no argument specified, get current directory contents
    dir = opendir("./");
  }
  else {
    //check if specified directory exists, open it
    if((dir = opendir(args[1])) == NULL) {
      printf("No such directory %s.\n", args[1]);
      return 1;
    }
  }
  while((dirContents = readdir(dir)) != NULL) {
    printf(" %s ", dirContents->d_name);
  }
  printf("\n");
  closedir(dir);
  return 1;
}

int environ(char** args) {
  const char* env = getenv("PATH");
  if(env != NULL) {
    printf("%s\n", env);
  }
  else {
    printf("getenv() returned NULL");
  }
  return 1;
}

int echo(char** args) {
  int i = 1;
  while(args[i] != NULL) {
    printf("%s ", args[i]);
    i++;
  }
  printf("\n");
  return 1;
}

//print out explanation of how to use the shell
int help(char** args) {
  //get height of window so we know how many lines to print at a time
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  int rows = w.ws_row - 1;  

  //open readme
  FILE* fp = fopen("readme", "r");
  //get length of readme file so array can be large enough to hold it
  fseek(fp, 0, SEEK_END);
  int len = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char text[len];
  char* input;
  int r = 1;
  //print first screen of readme file
  do {
    fgets(text, len, fp);
    printf("%s", text);
    r++;
  }
  while(text != NULL && r < rows);
  //now wait for user to hit enter or q to print next line or quit
  while(1) {
    input = readline();
    if(strcmp(input, "q") == 0) {
      return 1;
    }
    else {
      if(fgets(text, len, fp) == NULL) {
        break;
      }
      else {
        printf("%s", text);
      }
    }
  }

/*  while(text != NULL) {
    //print the first screen-ful of help page
    printf("%s", text);
    while(r < rows) {
      fgets(text, len, fp);
      printf("%s", text);
      r++;
    }
    //wait for enter cmd to read next line or q to quit
    input = readline();
    if(strcmp(input, "q") == 0) {
      return 1;
    }
    else {
      if(fgets(text, len, fp) == NULL) {
        break;
      }
      printf("%s", text);
    }
  }*/
  printf("Reached end of help file. Press enter to return to the shell.\n");
  //wait for any key press & enter to exit the function
  input = readline();
  return 1;
}

