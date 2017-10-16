#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

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
  //system("clear");
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
  int isRedirectIn = 0;
  int isRedirectOut = 0;
  int inBg = 0;
  int isPipe = 0;
  int pipeLoc;
  int exitStat, i, j;
  int stdinDup, stdoutDup, infile, outfile;
  int fd[2];

  //return without executing anything if user didn't type anything
  if(args[0] == NULL) {
    return ret;
  }

  //check if we need to run in background or redirect i/o
  i = 0;
  while(args[i] != NULL) {
   if(strcmp(args[i], "&") == 0) {
      inBg = 1;
    } 
   else if(strcmp(args[i], "<") == 0) {
      infile = open(args[i+1], O_RDONLY);
      args[i] = NULL;
      isRedirectIn = 1;
    }
    else if(strcmp(args[i], ">") == 0) {
      outfile = open(args[i+1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR | O_CLOEXEC);
      args[i] = NULL;
      isRedirectOut = 1;
    } 
    else if(strcmp(args[i], ">>") == 0) {
      outfile = open(args[i+1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR | O_CLOEXEC);
      args[i] = NULL;
      isRedirectOut = 1;
    }
    else if(strcmp(args[i], "|") == 0) {
      //--------------------------------------------------- this is what im working on now
      isPipe = 1;
    }
    i++;
  } 

  //check for builtin, then run matching fn if it is
  for(j = 0; j < 7; j++) { //j = number of builtin functions there are
    //check if first arg matches a builtin
    if(strcmp(args[0], builtins[j]) == 0) { 
      //save current i/o values so they can be restored later
      stdinDup = dup(STDIN_FILENO);
      stdoutDup = dup(STDOUT_FILENO);
      //redirect i/o if necessary
      if(isRedirectIn == 1) {
        dup2(infile, STDIN_FILENO);
      }
      if(isRedirectOut == 1) {
        dup2(outfile, STDOUT_FILENO);
      }
      //run function and set return value
      ret = (builtinFN[j])(args);
      //reset stdin/out to defaults
      dup2(stdoutDup, STDOUT_FILENO);
      dup2(stdinDup, STDIN_FILENO);
      //close files
      //close(infile);
      //close(outfile);
      isBuiltin = 1;
    }
  }

  //if we didn't find a builtin fn, fork and try to exec a file 
  if(isBuiltin == 0) {
    //create a pipe
/*
    int fd[2];
    if(pipe(fd) == -1) {
      printf("Pipe error\n");
    }
*/
    //FORK to create child
    pid_t pid;
    pid = fork();
    if(pid < 0) {
      printf("Error forking\n");
    }
    else if(pid == 0) {
      //in child
      //redirect stdin/out if needed
      if(isRedirectIn == 1) {
        dup2(infile, STDIN_FILENO);
      }
      if(isRedirectOut == 1) {
        dup2(outfile, STDOUT_FILENO);
      }
      //exec the invoked program
      execvp(args[0], args);// || 1;
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
    }
  }
  return ret;
}

//---------------------builtin functions------------------------------

/* quit this shell
*/
int quit(char** args) {
  return 0;
}


/* change current working directory
 */
int cd(char** args) {
  if(chdir(args[1]) != 0) {
    printf("No directory '%s' found.\n", args[1]);
  }
  return 1;
}

/* clear the terminal screen
*/
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


/* Print contents of current working directory
*/
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

/* Print current environment PATH strings
*/
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

/* Print [args]
*/
int echo(char** args) {
  int i = 1;
  while(args[i] != NULL) {
    printf("%s ", args[i]);
    i++;
  }
  printf("\n");
  return 1;
}

/*print readme file (explanation of how to use the shell)
*/
int help(char** args) {
  //open readme
  FILE* fp = fopen("readme", "r");
  if(fp == NULL) {
    printf("Can't find readme file.\n");
    return 1;
  }
  //get length of readme file so array can be large enough to hold it
  fseek(fp, 0, SEEK_END);
  int len = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char text[len];

  //if current stdout is the terminal, print some of readme file and then wait for user to hit enter to show more
  if(isatty(1)) {  
    //get height of window so we know how many lines to print at a time
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int rows = w.ws_row - 1;  
    
    char* input;
    int r = 1;
    //print first screen of readme file
    do {
      fgets(text, len, fp);
      printf("%s", text);
      r++;
    }
    while(text != NULL && r < rows);
    //now wait for user to hit enter to print next line or q to quit
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
          //get rid of trailing newline then print line
          strtok(text, "\n");
          printf("%s", text);
        }
      }
    }

    printf("Reached end of help file. Press enter to return to the shell.\n");
    //wait for any key press & enter to exit the function
    input = readline();
  }
  //STDOUT isn't the terminal, so just print the whole help file and don't wait for any user input
  else {
    while(fgets(text, len, fp) != NULL ) {
      printf("%s", text);
    }
  }

  return 1;
}

