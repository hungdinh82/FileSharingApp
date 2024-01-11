/*
* api for handle for action related file
* feature: search, original_path
*/
#ifndef __PATH_H__
#define __PATH_H__

void getListPath(char * path, char *listpath);
void getListFolder(char * path, char *listfolder);
void getListFile(char * path, char *listfile);
void remove_dir(char *path);
#endif

