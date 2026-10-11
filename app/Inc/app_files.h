#pragma once

#include "ff.h"

#define MAX_FILES_COUNT 50
#define MAX_FILE_PATH_LENGTH 50

typedef struct {
    char files[MAX_FILES_COUNT][MAX_FILE_PATH_LENGTH + 1];
    int count;
} Files;

typedef enum
{
    COPY_OK,
    COPY_IN_PROGRESS,
    COPY_ERR
} copy_status_t;

#ifdef __cplusplus
 extern "C" {
#endif

void FindWavFiles(const char *path, Files *files);
FRESULT FindFilesInDir(const char *path, Files *files);

FRESULT copy_file_between_drives(const char* src_path, const char* dst_path);


copy_status_t get_copy_status(void);

float get_copy_progress(void);

#ifdef __cplusplus
}
#endif