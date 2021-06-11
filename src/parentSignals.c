#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef myLibHeader
#define myLibHeader
#include "../lib/headers/myLib.h"
#endif

#ifndef travelMonitorClientUtilHeader
#define travelMonitorClientUtilHeader
#include "../lib/headers/travelMonitorClientUtil.h"
#endif

#define LOG_DIR "../log_folder/"
#define PURPLE  "\x1B[4;33m"    
#define RESET   "\x1B[0m"

extern int FLAGINTQUIT;
extern int FLAGCHLD;
extern int FLAGUSR2;

void parentInterrupt(int signo)
{
    if(signo==SIGINT){
        // printf("\nParent SIGINT             \n");
        fflush(stdout);
        FLAGINTQUIT = 1;
    }
    else if(signo==SIGQUIT){
        // printf("\nParent SIGQUIT            \n");
        fflush(stdout);
        FLAGINTQUIT = 1;
    }
    else if(signo==SIGCHLD){
        //printf("\rPARENT SIGCHLD            \n");
        fflush(stdout);
        int found = -1;
        int wstatus = 0;
        int pid = 0;
        int signal = -1;
        // while(pid<=0){
        if((pid=waitpid(-1, &wstatus, WNOHANG))==-1){
            perror("waitpid");
            // break;
        }
        // }
        if(pid!=-1)
            if(WIFSIGNALED(wstatus)){
                signal = WTERMSIG(wstatus);
                printf(PURPLE"Process %d was signaled %d\n"RESET,pid,signal);
                if(signal==9||signal==11){
                    printf(PURPLE"FLAGCHLD.\r\n"RESET);
                    FLAGCHLD = pid;
                }
            }
        // int wstatus = 0;
        // int pid = 0;
        // while(pid<=0){
        //     if((pid=waitpid(-1, &wstatus, WNOHANG))==-1){
        //         perror("waitpid");
        //         break;
        //     }
        // }
        // if(WIFSIGNALED(wstatus)){
        //     printf("Process %d was signaled %d\n",pid,WTERMSIG(wstatus));
        //     FLAGCHLD = pid;
        // }
        // // if(WIFEXITED(wstatus)){
        // //     printf("Process %d exited %d\n",pid,WEXITSTATUS(wstatus));
        // //     FLAGCHLD = 0;
        // // }
    }
    else if(signo==SIGUSR2){
        //printf(PURPLE"PARENT SIGUSR2.\n"RESET);
        FLAGUSR2++;
    }
}

int checkFlag(parentDataPointer **UniversalPtr)
{
    parentDataPointer *Universal = (*UniversalPtr);
    int numMonitors = *(Universal->numMonitorsPtr);
    Monitor **monitor = *(Universal->monitorPtr);
    char **argument = *(Universal->argumentPtr);
    usleep_hook(1100); //yparxei gia na dwsei xrono sto signal na orisei to flag se periptwsh pou topothetithei amesws meta apo kill(pid)
    // printf("flgintquit =%d flagchld=%d\n",FLAGINTQUIT,FLAGCHLD);   
    if(FLAGUSR2)
    {
        if(FLAGUSR2>=numMonitors){
            FLAGUSR2=0;
            return 2;
        }else
            return 0;
    }
    if(FLAGINTQUIT)
    {
        exitNow(UniversalPtr,0);
    }
    else if(FLAGCHLD)
    {
        int found = -1;
        // int wstatus = 0;
        // int pid = 0;
        // while(1)
        // {
        //     found = -1;
        //     wstatus = 0;
        //     pid = 0;
        //     while(pid<=0){
        //         if((pid=waitpid(-1, &wstatus, WNOHANG))==-1){
        //             perror("waitpid");
        //             break;
        //         }
        //     }
        //     if(pid==-1)
        //         break;
        //     if(WIFSIGNALED(wstatus)){
        //         printf("Process %d was signaled %d\n",pid,WTERMSIG(wstatus));
        //         FLAGCHLD = pid;
        //     }
        //     // if(WIFEXITED(wstatus)){
        //     //     printf("Process %d exited %d\n",pid,WEXITSTATUS(wstatus));
        //     //     FLAGCHLD = 0;
        //     // }
            for(int i=0;i<numMonitors;i++)
                if(monitor[i]->pid==FLAGCHLD){
                    found = i;
                }
            if(found!=-1)
            {
                printf(PURPLE"Process %d %d was terminated\n"RESET,found,FLAGCHLD);
                fork_monitor(Universal, found);
                passInfo(Universal, found);
                distributeSubdirs(Universal, found);
                usleep_hook(110000);    //while monitor proccesses create the datatypes  
                receiveBloomFilter(Universal, found);
                // act.sa_handler=SIG_IGN; 
                // sigaction (SIGCHLD,&act,NULL);
                kill(monitor[found]->pid,SIGCONT);
                while(!receiveReadyMsg(Universal, found))
                    kill(monitor[found]->pid,SIGCONT);
                kill(monitor[found]->pid,SIGCONT);
                // act.sa_handler=parentInterrupt; 
                // sigaction (SIGCHLD,&act,NULL);
            }
        // }
        FLAGCHLD = 0;
        return 0;
    }
    else 
        return 0;

}
