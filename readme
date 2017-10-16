----------MYSHELL MANUAL----------
 To access this manual from the shell: "help"
 Press enter to read the next line of the manual, or press q then enter to quit and return to the shell.
----------------------------------
 Contents: 
   1. Builtin Commands
   2. Executable Files
   3. Input/Output Redirection
   4. Piping
   5. Author Information
----------------------------------
 1.Builtin Commands
    cd [directory]
       Changes current working directory to the one with the relative path specified in the [directory] argument.
    clear
       Clears the screen.
    dir [directory]
       Prints a list of the contents of the specified directory. If no directory is specified, prints contents of current working directory.
    environ
       Prints current environment PATH strings.
    echo [comment]
       Prints [comment] to the screen.
    help
       Prints the contents of this manual.
    quit 
       Exits this shell.
----------------------------------
 2.Executable Files
    To run an executable from this shell, type the path to the executable and hit enter.
    An executable can be run in the background by adding the argument '&' after the path. The & must be separated by a space character.
----------------------------------
 3.Input/Output Redirection
    MyShell accepts i/o redirection commands.
    > [outputfile]   redirects output from the screen to [outputfile], replacing the existing file if it exists. Usable on internal commands dir, environ, echo, and help, and any executable files.
    >> [outputfile]  redirects output from the screen to [outputfile], appending to the existing file if it exists. Usable on internal commands dir, environ, echo, and help, and any executable files.
    < [inputfile]    redirects input from the keyboard to [inputfile]. Usable on any executable files.
----------------------------------
 4.Piping
    Multiple operations can be strung together using the pipe (|) operator. For example,
	 [program1] | [program2]
	will execute program1, then pass its output as the input to program2. This is the same as 
	 [program1] > somefile
	 [program2] < somefile
	Multiple commands can be piped together. The first command/executable must give some output, and the second command/executable must take input as a parameter.
----------------------------------
 5.Author: Elizabeth Munz
   Created for Temple University
               CIS 3207
               October 2017
----------------------------------
