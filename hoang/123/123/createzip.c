#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <zip.h>

void add_folder_to_zip(zip_t *zip_archive, const char *folder_path, const char *zip_entry_prefix) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(folder_path);
    if (!dir) {
        perror("Error opening folder");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            // Regular file, add to zip
            char file_path[PATH_MAX];
            snprintf(file_path, sizeof(file_path), "%s/%s", folder_path, entry->d_name);

            zip_source_t *zip_source = zip_source_file(zip_archive, file_path, 0, 0);
            if (zip_source) {
                char zip_entry_name[256];
                snprintf(zip_entry_name, sizeof(zip_entry_name), "%s%s", zip_entry_prefix, entry->d_name);
                zip_file_add(zip_archive, zip_entry_name, zip_source, ZIP_FL_OVERWRITE);
            } else {
                perror("Error creating zip source for file");
            }
        } else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // Directory, recurse into it
            char subfolder_path[PATH_MAX];
            snprintf(subfolder_path, sizeof(subfolder_path), "%s/%s", folder_path, entry->d_name);

            char subfolder_zip_entry_prefix[256];
            snprintf(subfolder_zip_entry_prefix, sizeof(subfolder_zip_entry_prefix), "%s%s/", zip_entry_prefix, entry->d_name);

            add_folder_to_zip(zip_archive, subfolder_path, subfolder_zip_entry_prefix);
        }
    }

    closedir(dir);
}

int main() {
    const char *zip_filename = "example_folder.zip";
    const char *folder_to_zip = "FileServer";

    // Create a new zip archive
    zip_t *zip_archive = zip_open(zip_filename, ZIP_CREATE | ZIP_TRUNCATE, NULL);
    if (!zip_archive) {
        perror("Error creating zip archive");
        return 1;
    }

    // Add the entire folder to the zip archive
    add_folder_to_zip(zip_archive, folder_to_zip, "");

    // Close the zip archive
    zip_close(zip_archive);

    printf("Zip file created successfully: %s\n", zip_filename);

    return 0;
}
