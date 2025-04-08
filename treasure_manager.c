#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>



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


void add (char *huntID)
{
    if (!dir_exists (huntID))
    {
        if (mkdir (huntID, 0777) != 0)
        {
            perror ("Eroare la crearea directorului!\n");
            return;
        }
    }
    char filePath[256];
    snprintf (filePath, sizeof (filePath), "%s/treasures.dat", huntID);
    int file;
    if ((file = open (filePath, O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0) //0644 - drepturi de citire si scriere pentru owner si doar de citire pentru ceilalti
    {
        perror ("Eroare la deschiderea fisierului!\n");
        return;
    }
    treasure_t treasure;
    printf ("Introduceti ID-ul: ");
    scanf ("%d", &treasure.ID);
    printf ("Introduceti User Name-ul: ");
    scanf ("%s", treasure.userName);
    printf ("Introduceti latitudinea: ");
    scanf ("%f", &treasure.latitude);
    printf ("Introduceti longitudinea: ");
    scanf ("%f", &treasure.longitude);
    printf ("Introduceti valoarea: ");
    scanf ("%d", &treasure.value);
    
    getchar();
    printf ("Introduceti clue-ul: ");
    fgets (treasure.clue, MAX_CLUE, stdin);
    treasure.clue[strcspn (treasure.clue, "\n")] = '\0';

    int size = write (file, &treasure, sizeof (treasure_t));
    if (size != sizeof (treasure_t))
    {
        perror ("Eroare la scrierea in fisier!\n");
    }
}


void list (char* huntID)
{
    char treasureFile[256];
    snprintf (treasureFile, sizeof (treasureFile), "%s/treasures.dat", huntID);

    struct stat st;
    if (stat (treasureFile, &st) != 0)
    {
        perror ("Eroare accesare fisier!\n");
        return;
    }
    int file;
    if ((file = open (treasureFile, O_RDONLY)) < 0)
    {
        perror ("Eroare deschidere fisier!\n");
        return;
    }
    treasure_t treasure;
    int size = read (file, &treasure, sizeof (treasure_t));
    if (size != sizeof (treasure_t))
    {
        perror ("Eroare citire fisier!\n");
        return;
    }
    while (size == sizeof (treasure_t))
    {
        printf ("%d\n%s\n%.2f\n%.2f\n%s\n%d\n", treasure.ID, treasure.userName, treasure.latitude, treasure.longitude, treasure.clue, treasure.value);
        size = read (file, &treasure, sizeof (treasure_t));
    }
    
    
}


void view (char* huntID, int ID)
{
    char treasureFile[256];
    snprintf (treasureFile, sizeof (treasureFile), "%s/treasures.dat", huntID);

    struct stat st;
    if (stat (treasureFile, &st) != 0)
    {
        perror ("Eroare accesare fisier!\n");
        return;
    }
    int file;
    if ((file = open (treasureFile, O_RDONLY)) < 0)
    {
        perror ("Eroare deschidere fisier!\n");
        return;
    }
    treasure_t treasure;
    int size = read (file, &treasure, sizeof (treasure_t));
    if (size != sizeof (treasure_t))
    {
        perror ("Eroare citire fisier!\n");
        return;
    }
    while (size == sizeof (treasure_t))
    {
        if (treasure.ID == ID)
        {
            printf ("%d\n%s\n%.2f\n%.2f\n%s\n%d\n", treasure.ID, treasure.userName, treasure.latitude, treasure.longitude, treasure.clue, treasure.value);
        }
        size = read (file, &treasure, sizeof (treasure_t));
    }
}


int main (void)
{
    //add("Hunt1");
    //list("Hunt1");
    view("Hunt1", 1);
    return 0;
}