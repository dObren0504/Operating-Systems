#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

pid_t monitor_pid = -1;
int monitor_running = 0;

void handler_sigusr1()
{

}

void handler_term()
{

}

void start_monitor ()
{
    if (monitor_running)
    {
        write (1, "Monitor is already running\n", strlen ("Monitor is already running\n"));
        return;
    }
    pid_t pid = fork();
    if (pid < 0)
    {
        perror ("Failed to fork");
    }
    if (pid == 0)
    {
        char message [256];
        snprintf (message, sizeof (message), "Monitor started, PID: %d\n", getpid());
        write (1, message, strlen(message));


        struct sigaction sa_term, sa_usr1;
        sa_usr1.sa_handler = handler_sigusr1;
        sigemptyset(&sa_usr1.sa_mask);  //initializes the signal mask to empty, meaning that no signals will be blocked during handler_sigusr1
        sa_usr1.sa_flags = 0;  //no special flags used here
        sigaction(SIGUSR1, &sa_usr1, NULL);  //after this, when SIGUSR1 is received, the process will call handler_sigusr1


        sa_term.sa_handler = handler_term;
        sigemptyset(&sa_term.sa_mask);  
        sa_term.sa_flags = 0;  
        sigaction(SIGTERM, &sa_term, NULL);

        while (1)
        {
            pause();  //pause the process until it receives a signal
        }
    }

    monitor_pid = pid;
    monitor_running = 1;

}

int main (void)
{
    start_monitor();
    return 0;
}

