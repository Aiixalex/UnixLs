#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <errno.h>

void _ls (char* dir_name, int* ops_map)
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

        if (!ops_map['a'] && (filename[0] == '.')) { // hidden files
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
    int ops_map[128] = {0};

    char* arg;
    int i = 1;
    while ((arg = argv[i]) != NULL)
    {
        if (arg[0] == '-')
        {
            int j=1;
            while (arg[j] != '\0')
            {
                int op_ascii = arg[j];
                ops_map[op_ascii] = true;
                j++;
            }
        }
        else
        {
            _ls(arg, ops_map);
        }

        i++;
    }
}