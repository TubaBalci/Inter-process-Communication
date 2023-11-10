// Programmer : Tuba Sultan Balci
// 21/10/2023

#include <stdio.h>
#include <signal.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>


#define MAX_USER 5
#define MEMORY_SIZE 216 

void sig_handler(int signum) {
    //the "message detected" signal handled
    if (signum == SIGUSR1) 
    {
        // arrangments to attach shared memory location
        key_t key = ftok ("/home",79);
        int memId=shmget(key,MEMORY_SIZE,0666 | IPC_CREAT); 

        if(memId == -1)
        {
            perror("memory couldn't reserved");
            return ;
        }
        
        // read the message from memory and print out to the terminal
        char *memory = (char *) shmat(memId,(void *)0, 0);
        printf("%s\n", memory);
        shmdt(memory);
    }
    // the "Ctrl^C" (SIGINT) signal detected
    // or in case of terminal closed without terminating the program
    else if (signum == SIGINT || signum == SIGHUP) 
    {
        // arrangements to remove pid from shared pid memory 
        key_t pKey = ftok ("/home",17);
        int pMemID = shmget(pKey,sizeof(pid_t) * (MAX_USER + 1),0666 | IPC_CREAT); 
        pid_t *ids = (pid_t *)shmat(pMemID, (void *)0, 0);
        pid_t id = getpid();

        //remove the id value tha is the current process
        for (int i = 0; i < MAX_USER; i++) 
            if(ids[i] == id)
                ids[i] = 0;
        printf("\n");
        
        shmdt(ids);
        // exiting program
        exit(0);
    }
}

int main ( int argc , char *argv[]){
    // take user name argument from command line
    char* name = argv [1];
    printf("-- %s --\n", name);
    
    //creating the required keys to reserve a memory location
    key_t key = ftok ("/home",79); //key value for shared memory
    key_t pKey = ftok ("/home",17); // key value for shared pid array memory
    // So we can keep track of the process we should send signal


    int memId=shmget(key,MEMORY_SIZE,0666 | IPC_CREAT); // memory creation
    int pMemID = shmget(pKey,sizeof(pid_t) * (MAX_USER + 1) ,0666 | IPC_CREAT); // memory for pid array creation

    // error handling
    if(memId == -1)
    {
        perror("memory couldn't reserved xxxxx");
        return 1;
    }

    if(pMemID == -1)
    {
        perror("process id memory couldn't reserved");
        return 1;
    }


    char *memory = (char *) shmat(memId,(void *)0, 0); // attach memory segment to str pointer
    pid_t *ids = (pid_t *)shmat(pMemID, (void *)0, 0); // attach memory segment to pid array pointer
    
    int pNo = 0; // number of processes that has their id in the array

    // check if the memory has created for the first time by looking ids[MAX_USER]
    // we never get to use the ids[MAX_USER] id so we arrange it as a flag
    // if it contains a nonzero value, it have been created for the first time
    // because it comes with unmeaninful value and we initialize all values to zero
    // if it is zero, then we have arranged it already so we need to find which index we should put the current pid 
    if(ids[MAX_USER] != 0){
        for (int i = 0; i < MAX_USER+1; i++) 
            ids[i] = 0;
    }else{
        for (int i = 0; i < MAX_USER; i++) 
        {
            if(ids[i] != 0)
              pNo++; // find the appropriate index value for current pid in array
            else
                break;
        }
        //if all processes are running and one more process starts to run
        // we reset the pid array and giving it pNo = 0
        // for given conditions in the hw document this isn't possible
        if(pNo == MAX_USER)
        {
            pNo = 0;
            for (int i = 0; i < MAX_USER + 1 ; i++) 
                ids[i] = 0;
        }
    }

    pid_t id = getpid();
    ids[pNo] = id; //saving the current pid to correct index
    //arrange handle functions
    signal(SIGUSR1,sig_handler); 
    signal(SIGINT,sig_handler);
    signal(SIGHUP, sig_handler);
    

    // be on alert for any message input until termination of the code
    while(1){
        char message[100];
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = 0; // remove the newline character

        // write message with the sender info
        char sender[256] = "[";
        strcat(sender, name);
        strcat(sender, "]: ");
        strcat(sender, message);
        strcpy(memory, sender);

        //send signals for active processes 
        for (int i = 0; i < MAX_USER; i++) {
            if (ids[i] != 0 && ids[i] != id) 
              kill(ids[i], SIGUSR1);
        }
    }

    shmdt(memory);
    shmdt(ids);
    
    return 0;

}

