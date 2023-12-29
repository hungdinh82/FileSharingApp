#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "path.h"

/**
 * Lists all files and sub-directories  
 * considering path as base path.
 */
void getListPath(char *basePath, char *listpath)
{
    char path[1000];
    memset(path,'\0',1000);
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Unable to open directory stream
    if (!dir) return;
    printf("%s\n",path);
    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            printf("%s\n",path);
            printf("%s\n",dp->d_name);

            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            strcat(listpath, path);
            strcat(listpath, "\n");
            getListPath(path,listpath);
        }
    }
    closedir(dir);
}
/**
 * Lists all sub-directories path recursively 
 * considering path as base path.
 */
void getListFolder(char *basePath, char *listfolder)
{
    char path[1000];
    memset(path,'\0',1000);
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Unable to open directory stream
    if (!dir) return;
    printf("%s\n",path);
    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            if(dp->d_type == DT_DIR) {
                strcat(listfolder, path);
                strcat(listfolder, "\n");
            }
            getListFolder(path,listfolder);
        }
    }
    closedir(dir);
}

/**
 * Lists all files path
 * considering path as base path.
 */
void getListFile(char *basePath, char *listfile)
{
    char path[1000];
    memset(path,'\0',1000);
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    // Unable to open directory stream
    if (!dir) return;
    printf("%s\n",path);
    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            if(dp->d_type == DT_REG) {
                strcat(listfile, path);
                strcat(listfile, "\n");
            }
            getListFile(path,listfile);
        }
    }
    closedir(dir);
}
/**
 * Delete the folder and all the files in it
 * @param path 
 */
void remove_dir(char *path)
{
        struct dirent *entry = NULL;
        DIR *dir = NULL;
        dir = opendir(path);
        while((entry = readdir(dir))!= NULL)
        {   
                DIR *sub_dir = NULL;
                FILE *file = NULL;
                char abs_path[1000] = {0};
                if(*(entry->d_name) != '.')
                {   
                        sprintf(abs_path, "%s/%s", path, entry->d_name);
                        if(sub_dir = opendir(abs_path))
                        {   
                                closedir(sub_dir);
                                remove_dir(abs_path);
                        }   
                        else 
                        {   
                                if((file = fopen(abs_path, "r"))!= NULL)
                                {   
                                        fclose(file);
                                        remove(abs_path);
                        
                                }   
                        }   
                }   
        }   
        remove(path);
}