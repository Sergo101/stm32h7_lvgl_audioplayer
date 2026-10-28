#include "app_files.h"

#include "log.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include <string.h>

#define FILENAME_EXTENSION  ".wav"
#define FILENAME_EXTENSION_SIZE  (sizeof(FILENAME_EXTENSION) - 1)

copy_status_t status = 0;
float progress = 0.0f;
BYTE tmpbuffer[2048];
FIL fin, fout;

void FindWavFiles(const char *path, Files *files)
{
    files->count = 0;
    FRESULT res;

    DIR dir;
    res = f_opendir(&dir, path);
    if (res != FR_OK) 
    {
        log_error("Error: f_opendir\n");
        return;
    }

    while (files->count < MAX_FILES_COUNT - 1) 
    {
        FILINFO file_info;
        res = f_readdir(&dir, &file_info);
        if (res != FR_OK) 
        {
            log_error("Error: f_readdir\n");
            return;
        }

        if (file_info.fname[0] == '\0') 
        {
            break;
        }

        if (!(file_info.fattrib & AM_DIR)) 
        {
            size_t len = strlen(file_info.fname);
            if (len >= FILENAME_EXTENSION_SIZE && len <= MAX_FILE_PATH_LENGTH && strcmp(&file_info.fname[len - FILENAME_EXTENSION_SIZE], FILENAME_EXTENSION) == 0) {
                printf("Found: %s\n", file_info.fname);
                strncpy(files->files[files->count], file_info.fname, MAX_FILE_PATH_LENGTH);
                files->count++;
            }
        }
    }

    f_closedir(&dir);
}

FRESULT FindFilesInDir(const char *path, Files *files)
{
    files->count = 0;
    FRESULT res;

    DIR dir;
    res = f_opendir(&dir, path);
    if (res != FR_OK) 
    {
        log_error("Error: f_opendir\n");
        return res;
    }

    while (files->count < MAX_FILES_COUNT - 1) 
    {
        FILINFO file_info;
        res = f_readdir(&dir, &file_info);
        if (res != FR_OK) 
        {
            log_error("Error: f_readdir\n");
            return res;
        }

        if (file_info.fname[0] == '\0') 
        {
            break;
        }

        if (!(file_info.fattrib & AM_DIR)) 
        {
            printf("Found: %s\n", file_info.fname);
            strncpy(files->files[files->count], file_info.fname, MAX_FILE_PATH_LENGTH);
            files->count++;
        }
    }

    f_closedir(&dir);
    return FR_OK;
}


FRESULT copy_file_between_drives(const char* src_path, const char* dst_path) 
{
    FRESULT res;
    // Use a tmpbuffer size appropriate for your system's RAM and transfer speed
    UINT br, bw;

    progress = 0.0f;
    status = COPY_IN_PROGRESS;
    res = f_open(&fin, src_path, FA_READ);
    if (res != FR_OK) {
        printf("Error opening source file: %d\n", res);
        status = COPY_ERR;
        return 0;
    }

    // 2. Open/create destination file (e.g., "1:file.txt")
    res = f_open(&fout, dst_path, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        printf("Error opening destination file: %d\n", res);
        f_close(&fin); // Close the source file before returning
        status = COPY_ERR;
        return 0;
    }

    // 3 & 4. Read from source and write to destination in a loop
    for (;;) {
        res = f_read(&fin, tmpbuffer, sizeof(tmpbuffer), &br); // Read a chunk
        if (res != FR_OK|| br == 0) break; // Error or end of file

        res = f_write(&fout, tmpbuffer, br, &bw); // Write the chunk
        progress = (float)f_tell(&fout) / (float)f_size(&fout);
        if (res != FR_OK || bw < br) break; // Error or disk full
    }
    
    // Optional: Ensure all data is written to the destination volume
    f_sync(&fout); 

    // 5. Close both files
    f_close(&fin);
    f_close(&fout);

    if (res == FR_OK && br == 0) {
        printf("File copy successful\n");
        status = COPY_OK;
        return 0;
    } else {
        printf("File copy failed with result: %d\n", res);
        // You might want to f_unlink(&fout) the destination file on failure
        status = COPY_ERR;
        return 1;
    }
}

copy_status_t get_copy_status(void)
{
    return status;
}

float get_copy_progress(void)
{
    return progress;
}