#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_USERS 1000
#define MAXCLUE 250
#define MAXUSERNAME 30

typedef struct 
{
    char username[MAXUSERNAME];
    int score;
} UserScore;

typedef struct 
{
    int ID;
    char userName[MAXUSERNAME];
    float latitude;
    float longitude;
    char clue[MAXCLUE];
    int value;
} treasure_t;

int find_user(UserScore *users, int count, char *name) 
{
    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, name) == 0) {
            return i;
        }
    }
    return -1;
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        write(2, "Usage: calculate_score <HUNT_ID>\n", strlen("Usage: calculate_score <HUNT_ID>\n"));
        return 1;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/treasures.dat", argv[1]);

    int fd = open(filepath, O_RDONLY);
    if (fd < 0)
    {
        perror("Failed to open treasure file");
        return 1;
    }

    UserScore users[MAX_USERS];
    int user_count = 0;

    treasure_t treasure;
    int size;
    while ((size = read(fd, &treasure, sizeof(treasure_t))) == sizeof(treasure_t))
    {
        int index = find_user(users, user_count, treasure.userName);
        if (index == -1)
        {
            if (user_count >= MAX_USERS)
                break;
            strncpy(users[user_count].username, treasure.userName, MAXUSERNAME - 1);
            users[user_count].username[MAXUSERNAME - 1] = '\0';
            users[user_count].score = treasure.value;
            user_count++;
        }
        else
        {
            users[index].score += treasure.value;
        }
    }

    close(fd);

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Scores for %s:\n", argv[1]);
    write(1, buffer, strlen(buffer));

    for (int i = 0; i < user_count; i++) 
    {
        
        snprintf(buffer, sizeof(buffer), "%s: %d\n", users[i].username, users[i].score);
        write(1, buffer, strlen(buffer));
    }

    return 0;
}
