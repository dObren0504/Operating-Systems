#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>


#define COMMAND_FILE "/tmp/command.txt"  // I will write the command in this file, so i can manage the SIGUSR1 signal depending on the command
                                         


pid_t monitor_pid = -1;
int monitor_running = 0;
int monitor_shutting_down = 0;
int shut_down_error_printed = 0;

void handler_sigusr1(int sig)
{
    char buffer[256];
    int fd = open(COMMAND_FILE, O_RDONLY);
    if (fd >= 0) 
    {
        ssize_t size = read(fd, buffer, sizeof(buffer) - 1);
        if (size > 0) 
        {
            buffer[size] = '\0';

            char *args[10];  // assuming max 10 arguments including program name and NULL
            int arg_index = 0;

            args[arg_index++] = "./treasure_manager";

            char *token = strtok(buffer, " ");
            while (token && arg_index < 9) 
            {
                args[arg_index++] = token;
                token = strtok(NULL, " ");
            }
            args[arg_index] = NULL;  // execvp requires NULL-terminated array

            pid_t pid = fork();
            if (pid == 0) 
            {
                execvp(args[0], args);
                perror("execvp failed");
                exit(1);
            } 
            else if (pid > 0)
            {
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status)) 
                {
                    char message[128];
                    snprintf(message, sizeof(message), "Command exited with status %d\n", WEXITSTATUS(status));
                    write(1, message, strlen(message));
                } 
            } 
            else 
            {
                perror("fork failed");
            }
        }
        close(fd);
    } 
    else 
    {
        perror("Monitor failed to open command file");
    }
}


void handler_term(int sig)
{
    write(1, "Monitor shutting down...\n", strlen ("Monitor shutting down...\n"));
    unlink("/tmp/command.txt");
    usleep(10000000);  
    exit(0);
}

void handler_sigchld(int sig)
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (pid == monitor_pid)
        {
            if (WIFEXITED(status))
            {
                char message[256];
                snprintf(message, sizeof(message), "Monitor exited with code: %d\n", WEXITSTATUS(status));
                write(1, message, strlen(message));
            }
            monitor_pid = -1;
            monitor_running = 0;
            monitor_shutting_down = 0;
            shut_down_error_printed = 0;
        }
    }
}

void start_monitor ()
{
    if (monitor_shutting_down)
    {
        return;
    }
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
        
    }
    else
    {
        write (1, "No monitor is running\n", strlen ("No monitor is running\n"));
    }
}

void handle_shutdown_error() {   //Function for handling the error when giving a command while shutting down 
    if (!shut_down_error_printed) 
    {
        write(1, "Monitor is shutting down, please wait...\n", strlen("Monitor is shutting down, please wait...\n"));
        shut_down_error_printed = 1;  
    }
}

int main (void)
{
    char compile_cmd[] = "gcc -Wall -o treasure_manager treasure_manager.c";
    system (compile_cmd);
    char command[256];

    struct sigaction sa_chld;
    sa_chld.sa_handler = handler_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;  //restarts certain system calls (read or write) that are interrupted by this signal  and the sigchld won't be sent when the child is stopped
    sigaction(SIGCHLD, &sa_chld, NULL);

    while (1)
    {
        //write (1, ">>> ", strlen (">>> "));
        int size = read (0, command, sizeof (command) - 1);
        command[size] = '\0';
        if (command[size - 1] == '\n')
        {
            command[size - 1] = '\0';
        }
        
        if (monitor_shutting_down)
        {
            handle_shutdown_error();
        }

        
        if (strcmp (command, "start_monitor") == 0)
        {
            start_monitor();
        }
        else if (strcmp (command, "stop_monitor") == 0)
        {
            stop_monitor();
        }
        else if (strcmp (command, "exit") == 0)
        {
            if (monitor_shutting_down == 0)
            {
                if (monitor_running)
                    write (1, "Monitor is still running, you need to stop it before you exit!\n", strlen ("Monitor is still running, you need to stop it before you exit!\n"));
                else
                    break;
            }
            else
                continue;
        }
        else if (strcmp(command, "list_treasures") == 0 || strcmp(command, "view_treasure") == 0)
        {
            if (monitor_running)
            {
                if (monitor_shutting_down == 0)
                {
                    char option[16];
                    char huntID[256];
                    char buff[16];

                    if (strcmp(command, "list_treasures") == 0) {
                        strcpy(option, "--list");
                        write(1, "Give a Hunt ID: ", strlen("Give a Hunt ID: "));
                    } 
                    else if (strcmp(command, "view_treasure") == 0) {
                        strcpy(option, "--view");
                        write(1, "Give a Hunt ID: ", strlen("Give a Hunt ID: "));
                    }

                    
                    int size = read(0, huntID, sizeof(huntID) - 1);
                    huntID[size] = '\0';
                    if (huntID[size - 1] == '\n') huntID[size - 1] = '\0';

                    
                    if (strcmp(command, "view_treasure") == 0) {
                        write(1, "Give a treasure ID: ", strlen("Give a treasure ID: "));
                        size = read(0, buff, sizeof(buff) - 1);
                        buff[size] = '\0';
                    }

                    
                    int fd = open(COMMAND_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd >= 0)
                    {
                        write(fd, option, strlen(option));
                        write(fd, " ", strlen(" "));
                        write(fd, huntID, strlen(huntID));
                        
                        
                        if (strcmp(command, "view_treasure") == 0) {
                            write(fd, " ", strlen(" "));
                            write(fd, buff, strlen(buff));
                        }

                        close(fd);
                    }
                    else
                    {
                        perror("Failed to write to command file");
                    }

                    
                    write(1, (strcmp(command, "list_treasures") == 0) ? "Listing treasures...\n" : "Viewing treasure...\n", 
                        (strcmp(command, "list_treasures") == 0) ? strlen("Listing treasures...\n") : strlen("Viewing treasure...\n"));
                    kill(monitor_pid, SIGUSR1);
                }
                else
                {
                    continue;
                }
            }
            else
            {
                write(1, "Monitor is not running\n", strlen("Monitor is not running\n"));
            }
        }
        else
        {
            write (1, "Invalid Command\n\nTry:\nstart_monitor\nlist_hunts\nlist_treasures\nview_treasure\nstop_monitor\nexit\n\n", strlen ("Invalid Command\n\nTry:\nstart_monitor\nlist_hunts\nlist_treasures\nview_treasure\nstop_monitor\nexit\n\n"));
        }
        
    }
    return 0;
}

