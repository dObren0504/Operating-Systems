#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>



#define MAX_CLUE 250
#define MAX_USERNAME 30


int dir_exists (char* path)
{
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR (st.st_mode));
}


typedef struct treasure
{
    int ID;
    char userName[MAX_USERNAME];
    float latitude;
    float longitude;
    char clue[MAX_CLUE];
    int value;
}treasure_t;



void log_hunt (char* huntID, char* message)
{
    char logFile[256];
    snprintf (logFile, sizeof (logFile), "%s/logged_hunt.txt", huntID);

    FILE* file = NULL;
    if ((file = fopen (logFile, "a")) == NULL)
    {
        perror ("Error opening file\n");
        return;
    }
    fprintf (file, "%s\n", message);
    fclose (file);

}



void add (char *huntID)
{
    if (!dir_exists (huntID))
    {
        if (mkdir (huntID, 0777) != 0) //0777 - read, write, execute for everybody
        {
            perror ("Error creating directory\n");
            return;
        }
    }
    char filePath[256];
    snprintf (filePath, sizeof (filePath), "%s/treasures.dat", huntID);
    int file;
    if ((file = open (filePath, O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0) //0644 - reading and writing rights for owner and only reading for others
    {
        perror ("Error opening file\n");
        return;
    }
    treasure_t treasure;
    char buffer[100];

    printf ("Enter the ID: ");
    fflush (stdout);
    read(0, buffer, sizeof(buffer));
    treasure.ID = atoi(buffer);

    printf ("Enter the User Name: ");
    fflush (stdout);
    read(0, treasure.userName, sizeof(treasure.userName));
    treasure.userName[strcspn(treasure.userName, "\n")] = '\0';

    printf ("Enter the latitude: ");
    fflush (stdout);
    read(0, buffer, sizeof(buffer));
    treasure.latitude = atof(buffer);

    printf ("Enter the longitude: ");
    fflush (stdout);
    read(0, buffer, sizeof(buffer));
    treasure.longitude = atof(buffer);

    printf ("Enter the value: ");
    fflush (stdout);
    read(0, buffer, sizeof(buffer));
    treasure.value = atoi(buffer);
    
    
    printf ("Enter the clue: ");
    fflush (stdout);
    read (0, treasure.clue, sizeof (treasure.clue));
    treasure.clue[strcspn (treasure.clue, "\n")] = '\0';

    int size = write (file, &treasure, sizeof (treasure_t));
    if (size != sizeof (treasure_t))
    {
        perror ("Error writing to file\n");
    }
    log_hunt (huntID, "Treasure added");
    close(file);
}





void list (char* huntID)
{
    char treasureFile[256];
    snprintf (treasureFile, sizeof (treasureFile), "%s/treasures.dat", huntID);

    struct stat st;
    if (stat (treasureFile, &st) != 0)
    {
        perror ("Error accessing file\n");
        return;
    }


    printf ("Hunt Name: %s\n", huntID);
    printf ("File size: %ld bytes\n", st.st_size);

    char timeBuffer[100];
    struct tm* time = localtime (&st.st_mtime);
    strftime (timeBuffer, sizeof (timeBuffer), "%Y-%m-%d %H:%M:%S", time);
    printf ("Last modified: %s\n", timeBuffer);

    int file;
    if ((file = open (treasureFile, O_RDONLY)) < 0)
    {
        perror ("Error opening file\n");
        return;
    }
    treasure_t treasure;
    int size = read (file, &treasure, sizeof (treasure_t));
    if (size != sizeof (treasure_t))
    {
        perror ("Error reading file\n");
        return;
    }
    while (size == sizeof (treasure_t))
    {
        printf ("%d\n%s\n%.2f\n%.2f\n%s\n%d\n", treasure.ID, treasure.userName, treasure.latitude, treasure.longitude, treasure.clue, treasure.value);
        size = read (file, &treasure, sizeof (treasure_t));
    }
    log_hunt (huntID, "Listed treasures");
    close (file);
    
}


void view (char* huntID, int ID)
{
    char treasureFile[256];
    snprintf (treasureFile, sizeof (treasureFile), "%s/treasures.dat", huntID);

    struct stat st;
    if (stat (treasureFile, &st) != 0)
    {
        perror ("Error accessing file\n");
        return;
    }
    int file;
    if ((file = open (treasureFile, O_RDONLY)) < 0)
    {
        perror ("Error opening file\n");
        return;
    }
    treasure_t treasure;
    int size = read (file, &treasure, sizeof (treasure_t));
    if (size != sizeof (treasure_t))
    {
        perror ("Error reading file\n");
        return;
    }
    char message[256];
    int count = 0;
    while (size == sizeof (treasure_t))
    {
        if (treasure.ID == ID)
        {
            printf ("%d\n%s\n%.2f\n%.2f\n%s\n%d\n", treasure.ID, treasure.userName, treasure.latitude, treasure.longitude, treasure.clue, treasure.value);
            snprintf (message, sizeof (message), "Viewed treasure with ID: %d", treasure.ID);
            count++;
        }
        size = read (file, &treasure, sizeof (treasure_t));
    }
    if (count == 0)
    {
        perror ("Treasure not found\n");
        return;
    }
    log_hunt (huntID, message);
    close (file);
}




int main (void)
{
    printf ("Choose your option\n");
    printf ("Option 1: Add a treasure\n");
    printf ("Option 2: List a hunt\n");
    printf ("Option 3: View a specific treasure\n");
    printf ("Option 4: Remove a treasure\n");
    printf ("Option 5: Remove a hunt\n\n");
    int option;
    printf ("Enter your option number: ");
    scanf ("%d", &option);
    switch (option)
    {
        char huntName[100];
        int ID;
        case 1:
            printf ("Enter a hunt name: ");
            scanf ("%s", huntName);
            add (huntName);
            break;
        case 2: 
            printf ("Enter a hunt name: ");
            scanf ("%s", huntName);
            list (huntName);
            break;
        case 3:
            printf ("Enter a hunt and a specific ID: ");
            scanf ("%s %d", huntName, &ID);
            view (huntName, ID);
            break;
        default:
            perror ("Invalid option\n");
            break;
    }
    
    return 0;
}