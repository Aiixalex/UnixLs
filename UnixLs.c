#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <errno.h>

void _ls (char* dir_name, bool op_a)
{
    DIR *dirp = opendir(dir_name);
    if (dirp == NULL)
    {
        perror(dir_name);
        exit(EXIT_FAILURE);
    }

    errno = 0; // To distinguish end of stream from an error
    struct dirent *direntp;
    while ((direntp = readdir(dirp)) != NULL) {
        char *filename = direntp->d_name;

        if (!op_a && (filename[0] == '.')) { // hidden files
            continue;
        }
        
        printf("%s\n", filename);
    }

    if (errno != 0)
    {
        perror("Unable to read filename");
    }
}

int main (int argc, char* argv[]) {
    _ls(argv[1], false);
}