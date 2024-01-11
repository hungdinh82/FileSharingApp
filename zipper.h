#ifndef ZIPPER_H
#define ZIPPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libgen.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <zip.h>

void add_folder_to_zip(zip_t* zip_archive, const char* folder_path, const char* zip_entry_prefix);

int create_zip(char* filename, char* folder);

void rec_mkdir(const char* dir);

int extract_zip(const char* zip_filename, const char* extract_path);

int is_folder_empty(const char* folder_path);

#endif // !ZIPPER_H
