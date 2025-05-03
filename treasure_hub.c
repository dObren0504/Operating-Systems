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
int monitor_shutting_down = 0;

void handler_sigusr1(int sig)
{

}

void handler_term(int sig)
{
    write(1, "Monitor shutting down...\n", strlen ("Monitor shutting down...\n"));
    usleep(10000000);  
    exit(0);
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
    else
    {
        char message [256];
        snprintf (message, sizeof (message), "Monitor started, PID: %d\n", pid);
        write (1, message, strlen(message));
        monitor_pid = pid;
        monitor_running = 1;
    }

}

void stop_monitor ()
{
    if (monitor_running && monitor_pid > 0)
    {
        monitor_shutting_down = 1;
        kill (monitor_pid, SIGTERM);
        int status;
        waitpid (monitor_pid, &status, 0);
        if (WIFEXITED(status))
        {
            {
                char message[256];
                snprintf(message, sizeof(message), "Monitor exited with code: %d\n", WEXITSTATUS(status));
                write(1, message, strlen(message));
            }
            monitor_pid = -1;
            monitor_running = 0;
            monitor_shutting_down = 0;
        }
    }
    else
    {
        write (1, "No monitor is running\n", strlen ("No monitor is running\n"));
    }
}

int main (void)
{
    char compile_cmd[] = "gcc -Wall -o treasure_manager treasure_manager.c";
    system (compile_cmd);
    char command[256];
    while (1)
    {
        

        int size = read (0, command, sizeof (command) - 1);
        command[size] = '\0';
        if (command[size - 1] == '\n')
        {
            command[size - 1] = '\0';
        }
        if (monitor_shutting_down) {
            write(1, "Monitor is shutting down, please wait...\n", strlen("Monitor is shutting down, please wait...\n"));
            continue;
        }
        if (strcmp (command, "start_monitor") == 0)
        {
            start_monitor();
        }
        else if (strcmp (command, "stop_monitor") == 0)
        {
            stop_monitor();
        }
    }
    return 0;
}

