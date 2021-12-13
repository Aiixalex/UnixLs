#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <uuid/uuid.h>

#define OPTIONS_NUM_MAX 128
#define DIRS_NUM_MAX 64

void _ls (char* dir_name, int* ops_map)
{
    DIR *dirp = opendir (dir_name);
    if (dirp == NULL)
    {
        perror (dir_name);
        exit (EXIT_FAILURE);
    }

    errno = 0; // To distinguish end of stream from an error
    struct dirent *direntp;
    while ((direntp = readdir(dirp)) != NULL) {
        char *filename = direntp->d_name;

        size_t dir_name_len = strlen (dir_name);
        size_t filename_len = strlen (filename);
        char filepath[dir_name_len + filename_len + 2];
        memcpy (filepath, dir_name, dir_name_len);
        memcpy (filepath + dir_name_len, "/", 1);
        memcpy (filepath + dir_name_len + 1, filename, filename_len + 1);

        if (!ops_map['a'] && (filename[0] == '.')) { // hidden files
            continue;
        }

        if (ops_map['i'])
        {
            unsigned long ino = direntp->d_ino;
            printf ("%lu ", ino);
        }

        if (ops_map['l'])
        {
            struct stat file_stat;

            int res = lstat (filepath, &file_stat);
            if (res == -1)
            {
                perror (filepath);
                exit (EXIT_FAILURE);
            }

            uid_t uid = file_stat.st_uid;
            struct passwd *pw = getpwuid (uid);
            if (pw == NULL)
            {
                printf("No name found for %u\n", uid);
                exit (EXIT_FAILURE);
            }

            char* user_name = pw->pw_name;
            printf ("%s ", user_name);

            gid_t gid = file_stat.st_gid;
            struct group *grp = getgrgid (gid);
            if (grp == NULL)
            {
                printf ("No group name for %u found\n", gid);
            }
            
            char* grp_name = grp->gr_name;
            printf ("%s ", grp_name);

            off_t file_sz = file_stat.st_size;
            printf ("%5ld ", file_sz);
        }

        // is symlink
        if (direntp->d_type == DT_LNK)
        {
            char symlink_path[PATH_MAX];
            char *res = realpath (filepath, symlink_path);
            if (res != NULL)
            {
                printf ("%s -> %s\n", filename, symlink_path);
            }
        }
        else
        {
            printf ("%s\n", filename);
        }
    }

    if (errno != 0)
    {
        perror ("Unable to read filename");
    }
}

int main (int argc, char* argv[]) {
    int ops_map[OPTIONS_NUM_MAX] = {0};

    char* dirs[DIRS_NUM_MAX];
    int dirs_idx = 0;

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
            dirs[dirs_idx] = arg;
            dirs_idx++;
        }

        i++;
    }

    if (dirs_idx == 0) // default ls
    {
        _ls (".", ops_map);
    }
    else if (dirs_idx == 1)
    {
        _ls (dirs[0], ops_map);
    }
    else
    {
        for (int k=0; k<dirs_idx; k++)
        {
            printf("%s:\n", dirs[k]);

            _ls (dirs[k], ops_map);

            if (k != dirs_idx - 1)
            {
                printf("\n");
            }
        }
    }
}