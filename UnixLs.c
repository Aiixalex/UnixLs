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
#define FOLDERS_NUM_MAX 128

const char* int2mon[12] = {"Mon", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void _ls (char* dir_name, int* ops_map)
{
    DIR *dirp = opendir (dir_name);
    if (dirp == NULL)
    {
        perror (dir_name);
        exit (EXIT_FAILURE);
    }

    char* folder_paths[FOLDERS_NUM_MAX];
    int folder_idx = 0;

    struct dirent *direntp;
    while ((direntp = readdir(dirp)) != NULL) {
        char *filename = direntp->d_name;

        struct stat file_stat;

        // generate the path of files in dir
        size_t dir_name_len = strlen (dir_name);
        size_t filename_len = strlen (filename);
        char* filepath = malloc(dir_name_len + filename_len + 2);

        memcpy (filepath, dir_name, dir_name_len);
        memcpy (filepath + dir_name_len, "/", 1);
        memcpy (filepath + dir_name_len + 1, filename, filename_len + 1);

        // -a: show all files (including hidden files)
        if (!ops_map['a'] && (filename[0] == '.')) {
            continue;
        }

        // -i: print inode number
        if (ops_map['i'])
        {
            unsigned long ino = direntp->d_ino;
            printf ("%lu ", ino);
        }

        // -l: use long listing format
        if (ops_map['l'])
        {
            int res = lstat (filepath, &file_stat);
            if (res == -1)
            {
                perror (filepath);
                exit (EXIT_FAILURE);
            }

            // print file permission
            if (S_ISDIR(file_stat.st_mode))
            {
                printf("d");
            }
            else if (S_ISLNK(file_stat.st_mode))
            {
                printf("l");
            }
            else
            {
                printf("-");
            }

            printf ((file_stat.st_mode & S_IRUSR) ? "r" : "-");
            printf ((file_stat.st_mode & S_IWUSR) ? "w" : "-");
            printf ((file_stat.st_mode & S_IXUSR) ? "x" : "-");
            printf ((file_stat.st_mode & S_IRGRP) ? "r" : "-");
            printf ((file_stat.st_mode & S_IWGRP) ? "w" : "-");
            printf ((file_stat.st_mode & S_IXGRP) ? "x" : "-");
            printf ((file_stat.st_mode & S_IROTH) ? "r" : "-");
            printf ((file_stat.st_mode & S_IWOTH) ? "w" : "-");
            printf ((file_stat.st_mode & S_IXOTH) ? "x" : "-");
            printf (" ");

            // print the number of links or directories inside the directory
            printf ("%lu ", file_stat.st_nlink);

            // print user id
            uid_t uid = file_stat.st_uid;
            struct passwd *pw = getpwuid (uid);
            if (pw == NULL)
            {
                printf("No name found for %u\n", uid);
                exit (EXIT_FAILURE);
            }

            char* user_name = pw->pw_name;
            printf ("%s ", user_name);

            // print group id
            gid_t gid = file_stat.st_gid;
            struct group *grp = getgrgid (gid);
            if (grp == NULL)
            {
                printf ("No group name for %u found\n", gid);
            }
            
            char* grp_name = grp->gr_name;
            printf ("%s ", grp_name);

            //  print total size of file (bytes)
            off_t file_sz = file_stat.st_size;
            printf ("%5ld ", file_sz);

            // print last modification time
            struct timespec mtim = file_stat.st_mtim;
            time_t t = mtim.tv_sec;
            putenv("TZ=America/Vancouver");
            struct tm* time_ptr = localtime(&t);
            printf("%s %2d %4d %02d:%02d ", int2mon[time_ptr->tm_mon], time_ptr->tm_mday, time_ptr->tm_year + 1900, 
                time_ptr->tm_hour, time_ptr->tm_min);
        }

        // is symlink
        if (S_ISLNK(file_stat.st_mode))
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

        if ((S_ISDIR(file_stat.st_mode)) && (strcmp(filename, ".") != 0) && (strcmp(filename, "..") != 0))
        {
            folder_paths[folder_idx] = filepath;
            folder_idx++;
        }
    }
    
    // -R: recursively ls
    if (ops_map['R'])
    {
        if (folder_idx != 0)
        {
            for (int i=0; i<folder_idx; i++)
            {
                printf("\n%s:\n", folder_paths[i]);
                
                _ls (folder_paths[i], ops_map);
                // free(folder_paths[i]);
            }
        }
        
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