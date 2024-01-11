#include "zipper.h"

void add_folder_to_zip(zip_t* zip_archive, const char* folder_path, const char* zip_entry_prefix) {
    DIR* dir;
    struct dirent* entry;

    dir = opendir(folder_path);
    if (!dir) {
        perror("Error opening folder");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char file_path[PATH_MAX];
            snprintf(file_path, sizeof(file_path), "%s/%s", folder_path, entry->d_name);
            zip_source_t* zip_source = zip_source_file(zip_archive, file_path, 0, 0);
            if (zip_source) {
                char zip_entry_name[256];
                snprintf(zip_entry_name, sizeof(zip_entry_name), "%s%s", zip_entry_prefix, entry->d_name);
                zip_file_add(zip_archive, zip_entry_name, zip_source, ZIP_FL_OVERWRITE);
            } else {
                perror("Error creating zip source for file");
            }
        } else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char subfolder_path[PATH_MAX];
            snprintf(subfolder_path, sizeof(subfolder_path), "%s/%s", folder_path, entry->d_name);
            char subfolder_zip_entry_prefix[256];
            snprintf(subfolder_zip_entry_prefix, sizeof(subfolder_zip_entry_prefix), "%s%s/", zip_entry_prefix, entry->d_name);
            add_folder_to_zip(zip_archive, subfolder_path, subfolder_zip_entry_prefix);
        }
    }

    closedir(dir);
}

int create_zip(char* filename, char* folder) {
    zip_t* zip_archive = zip_open(filename, ZIP_CREATE | ZIP_TRUNCATE, NULL);
    if (!zip_archive) {
        perror("Error creating zip archive");
        return 0;
    }

    add_folder_to_zip(zip_archive, folder, "");
    zip_close(zip_archive);
    return 1;
}

void rec_mkdir(const char* dir) {
    char tmp[256];
    char* p = NULL;
    size_t len;
    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/' || tmp[len - 1] == '\\') {
        tmp[len - 1] = 0;
    }

    for (p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    }
    mkdir(tmp, S_IRWXU);
}

int extract_zip(const char* zip_filename, const char* extract_path) {
    zip_t* zip_archive = zip_open(zip_filename, ZIP_RDONLY, NULL);
    if (!zip_archive) {
        perror("Error opening zip archive");
        return 0;
    }

    int num_entries = zip_get_num_entries(zip_archive, 0);
    if (num_entries < 0) {
        perror("Error getting number of entries");
        zip_close(zip_archive);
        return 0;
    }

    for (int i = 0; i < num_entries; i++) {
        zip_stat_t zip_stat;
        if (zip_stat_index(zip_archive, i, 0, &zip_stat) == 0) {
            zip_file_t* zip_file = zip_fopen_index(zip_archive, i, 0);
            if (zip_file) {
                char extract_filename[256];
                snprintf(extract_filename, sizeof(extract_filename), "%s/%s", extract_path, zip_stat.name);
                char extract_dir[256];
                strcpy(extract_dir, extract_filename);
                dirname(extract_dir);
                rec_mkdir(extract_dir);

                FILE* extracted_file = fopen(extract_filename, "wb");
                if (extracted_file) {
                    char buffer[4096];
                    zip_int64_t num_bytes = 0;
                    while ((num_bytes = zip_fread(zip_file, buffer, sizeof(buffer))) > 0) {
                        fwrite(buffer, 1, (size_t)num_bytes, extracted_file);
                    }
                    fclose(extracted_file);
                } else {
                    perror("Error opening extracted file for writing");
                }
            } else {
                perror("Error getting zip file entry info");
            }
        }
    }

    zip_close(zip_archive);
    return 1;
}
