#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>


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
    if ((file = open (filePath, O_WRONLY | O_CREAT | O_APPEND, 0777)) < 0) //0644 - drepturi de citire si scriere pentru owner si doar de citire pentru ceilalti
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
    scanf ("%e", &treasure.latitude);
    printf ("Introduceti longitudinea: ");
    scanf ("%e", &treasure.longitude);
    printf ("Introduceti valoarea: ");
    scanf ("%d", &treasure.value);
    
    getchar();
    printf ("Introduceti clue-ul: ");
    fgets (treasure.clue, MAX_CLUE, stdin);

    int size = write (file, &treasure, sizeof (treasure_t));
    if (size != sizeof (treasure_t))
    {
        perror ("Eroare la scrierea in fisier!\n");
    }
}


int main (void)
{
    add("Hunt1");
    return 0;
}