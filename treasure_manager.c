#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>


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



void log_hunt(char* huntID, char* message)
{
    char logFile[256];
    snprintf(logFile, sizeof(logFile), "%s/logged_hunt.txt", huntID);

    int file = open(logFile, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (file < 0)
    {
        perror("Error opening file");
        return;
    }

    char buffer[1024];
    int len = snprintf(buffer, sizeof(buffer), "%s\n", message);
    if (write(file, buffer, len) != len)
    {
        perror("Error writing to file");
    }

    close(file);
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
    char prompt[64];
    int len;

    len = snprintf(prompt, sizeof(prompt), "Enter the ID: ");
    write(1, prompt, len);
    read(0, buffer, sizeof(buffer));
    treasure.ID = atoi(buffer);

    len = snprintf(prompt, sizeof(prompt), "Enter the User Name: ");
    write(1, prompt, len);
    read(0, treasure.userName, sizeof(treasure.userName));
    treasure.userName[strcspn(treasure.userName, "\n")] = '\0';

    len = snprintf(prompt, sizeof(prompt), "Enter the latitude: ");
    write(1, prompt, len);
    read(0, buffer, sizeof(buffer));
    treasure.latitude = atof(buffer);

    len = snprintf(prompt, sizeof(prompt), "Enter the longitude: ");
    write(1, prompt, len);
    read(0, buffer, sizeof(buffer));
    treasure.longitude = atof(buffer);

    len = snprintf(prompt, sizeof(prompt), "Enter the value: ");
    write(1, prompt, len);
    read(0, buffer, sizeof(buffer));
    treasure.value = atoi(buffer);

    len = snprintf(prompt, sizeof(prompt), "Enter the clue: ");
    write(1, prompt, len);
    read (0, treasure.clue, sizeof (treasure.clue));
    treasure.clue[strcspn (treasure.clue, "\n")] = '\0';

    int size = write (file, &treasure, sizeof (treasure_t));
    if (size != sizeof (treasure_t))
    {
        perror ("Error writing to file\n");
    }
    log_hunt (huntID, "Treasure added");
    char logPath[256];
    snprintf (logPath, sizeof (logPath), "%s/logged_hunt.txt", huntID);

    char linkedPath[256];
    snprintf (linkedPath, sizeof (linkedPath), "logged_hunt-%s", huntID);
    if (symlink(logPath, linkedPath) == -1 && errno != EEXIST)
    {
        perror("Error creating symlink");
    }
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


    char output[512];
    int len;
    write (1, "===================\n", strlen ("===================\n"));
    len = snprintf(output, sizeof(output), "Hunt Name: %s\n", huntID);
    write(1, output, len);
    write (1, "-------------------\n", strlen ("-------------------\n"));
    len = snprintf(output, sizeof(output), "File size: %ld bytes\n", st.st_size);
    write(1, output, len);

    char timeBuffer[100];
    struct tm* time = localtime (&st.st_mtime);
    strftime (timeBuffer, sizeof (timeBuffer), "%Y-%m-%d %H:%M:%S", time);
    len = snprintf(output, sizeof(output), "Last modified: %s\n", timeBuffer);
    write(1, output, len);
    write (1, "-------------------\n", strlen ("-------------------\n"));
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
        len = snprintf(output, sizeof(output), "%d\n%s\n%.2f\n%.2f\n%s\n%d\n",
                       treasure.ID, treasure.userName, treasure.latitude,
                       treasure.longitude, treasure.clue, treasure.value);
        write(1, output, len);
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
            char buffer[512];
            snprintf(buffer, sizeof(buffer), "%d\n%s\n%.2f\n%.2f\n%s\n%d\n", 
                   treasure.ID, treasure.userName, treasure.latitude, 
                   treasure.longitude, treasure.clue, treasure.value);
            write(1, buffer, strlen (buffer));
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


void remove_treasure (char* huntID, int ID)
{
    char filePath[256];
    snprintf (filePath, sizeof (filePath), "%s/treasures.dat", huntID);
    int file;
    if ((file = open (filePath, O_RDONLY)) < 0)
    {
        perror ("Error opening file\n");
        return;
    }
    char copyPath[256];
    snprintf (copyPath, sizeof (filePath), "%s/copy.dat", huntID);
    int copyFile;
    if ((copyFile = open (copyPath, O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0) //0644 - reading and writing rights for owner and only reading for others
    {
        perror ("Error opening file\n");
        return;
    }
    treasure_t treasure;
    int count = 0;
    int size = read (file, &treasure, sizeof (treasure_t));
    if (size != sizeof (treasure_t))
    {
        perror ("Error reading file\n");
        return;
    }
    char message[256];
    while (size == sizeof (treasure_t))
    {
        if (treasure.ID == ID)
        {
            snprintf (message, sizeof (message), "Removed treasure with ID: %d", treasure.ID);
            count++;
        }
        else
        {
            int size = write (copyFile, &treasure, sizeof (treasure_t));
            if (size != sizeof (treasure_t))
            {
                perror ("Error writing to file\n");
            }
        }
        size = read (file, &treasure, sizeof (treasure_t));
    }
    if (count == 0)
    {
        perror ("Treasure not found\n");
        remove (copyPath);
        return;
    }
    else
    {
        log_hunt (huntID, message);
    }
    if (rename (copyPath, filePath) != 0)
    {
        perror ("Error renaming the file");
        return;
    }
    
    close (file);
    close (copyFile);
}


void remove_hunt (char* huntID)
{
    char filePath[256];
    snprintf (filePath, sizeof (filePath), "%s/treasures.dat", huntID);

    if (remove (filePath))
    {
        perror ("Error removing the file");
        return;
    }

    char logPath[256];
    snprintf (logPath, sizeof (logPath), "%s/logged_hunt.txt", huntID);

    if (remove (logPath))
    {
        perror ("Error removing the file");
        return;
    }

    char linkPath[256];
    snprintf (linkPath, sizeof (linkPath), "logged_hunt-%s", huntID);
    if (remove (linkPath))
    {
        perror ("Error removing symbolic link");
        return;
    }
    if (remove (huntID))
    {
        perror ("Error removing directory");
        return;
    }

}



void list_hunts() 
{
    DIR *dir = opendir(".");
    if (!dir) 
    {
        perror("Could not open current directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) 
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        struct stat st;
        if (stat(entry->d_name, &st) == 0 && S_ISDIR(st.st_mode)) 
        {
            char treasureFile[512];
            snprintf(treasureFile, sizeof(treasureFile), "%s/treasures.dat", entry->d_name);

            if (access(treasureFile, F_OK) == 0) //F_OK is a flag for checking only the existence
            {
                list(entry->d_name);
            }
        }
    }
    closedir(dir);
}



int main (int argc, char** argv)
{
    if (argc < 2)
    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "Usage:\n");
        write(2, buffer, strlen(buffer));   //2 is the file descriptor for stderr

        snprintf(buffer, sizeof(buffer), "  %s --add <huntName>\n", argv[0]);
        write(2, buffer, strlen(buffer));

        snprintf(buffer, sizeof(buffer), "  %s --list <huntName>\n", argv[0]);
        write(2, buffer, strlen(buffer));

        snprintf(buffer, sizeof(buffer), "  %s --view <huntName> <ID>\n", argv[0]);
        write(2, buffer, strlen(buffer));

        snprintf(buffer, sizeof(buffer), "  %s --remove <huntName> <ID>\n", argv[0]);
        write(2, buffer, strlen(buffer));

        snprintf(buffer, sizeof(buffer), "  %s --delete <huntName>\n", argv[0]);
        write(2, buffer, strlen(buffer));
        return 1;
    }

    char* option = argv[1];
    char* huntName = argv[2];

    if (strcmp (option, "--add") == 0)
    {
        add (huntName);
    }
    else if (strcmp (option, "--list") == 0)
    {
        list (huntName);
    }
    else if (strcmp (option, "--view") == 0)
    {
        if (argc < 4)
        {
            fprintf (stderr, "You need to enter a specific ID\n");
            return 1;
        }
        int ID = atoi (argv [3]);
        view (huntName, ID);
    }
    else if (strcmp (option, "--remove") == 0)
    {
        if (argc < 4)
        {
            fprintf (stderr, "You need to enter a specific ID\n");
            return 1;
        }
        int ID = atoi (argv[3]);
        remove_treasure (huntName, ID);
    }
    else if (strcmp (option, "--delete") == 0)
    {
        remove_hunt (huntName);
    }
    else if (strcmp (option, "--list_hunts") == 0)
    {
        list_hunts();
    }
    else
    {
        fprintf (stderr, "Invalid option\n");
        return 1;
    }

    return 0;
}