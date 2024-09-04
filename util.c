
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char* readFileToString(const char* filename) {
    // Open the file in binary mode to ensure no line-ending conversion happens
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the entire file content plus a null terminator
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    // Read the file into the buffer
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize) {
        perror("Failed to read the entire file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    // Null-terminate the string
    buffer[bytesRead] = '\0';

    // Close the file and return the string
    fclose(file);
    return buffer;
}

long getTime()
{
    // Get the number of clock ticks since the program started
    clock_t ticks = clock();

    // Convert clock ticks to milliseconds
    long milliseconds = (ticks * 1000) / CLOCKS_PER_SEC;

    return milliseconds;
}