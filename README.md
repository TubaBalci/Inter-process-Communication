# Inter-process-Communication
This program perform inter-process communication between 2 and 5 processes in C programming language. It takes a user name as enviromental variable when start running. And it prints the messages from other processes with usage of shared memory segment. To send a message write it on the terminal and press enter. The message will be displayed on other processes terminal.

To see actual communication you should run the code in different tabs at least twice. If you want to create more than 5 processes you should define beforehand.

# Dependencies
The code uses <stdio.h> <signal.h> <sys/shm.h> <string.h> <unistd.h> <sys/types.h><errno.h> <stdlib.h>
libraries of C language. Make sure these libraries exist in your machine.
