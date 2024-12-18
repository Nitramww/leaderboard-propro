#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

const char logFilename[] = "log.txt";

void log_file_append_with_uploaded_data(char *boundary, int contentLength);

// this program prints the post request data to web
int main(){
    printf("Content-Type: text/html; charset=utf-8\r\n\r\n");

    char *contentType = getenv("CONTENT_TYPE");
    printf("content type: %s<br>", contentType ? contentType : "Not provided");

    char *contentLengthStr = getenv("CONTENT_LENGTH");
    printf("content length: %s<br>", contentLengthStr ? contentLengthStr : "Not provided");
    int contentLength = atoi(contentLengthStr);

    char *boundary = strstr(contentType, "boundary=");
    if(!boundary){
        printf("Boundary not found in Content-Type.\n");
        return 1;
    }
    boundary += 9;  // Move past "boundary="

    log_file_append_with_uploaded_data(boundary, contentLength);

    return 0;
}

void log_file_append_with_uploaded_data(char *boundary, int contentLength){
    char buffer[BUFFER_SIZE] = {0};
    char filename[BUFFER_SIZE] = {0};
    char *fileStart, *fileEnd;
    int bytesRead = 0;
    FILE *outputFile;

    if(contentLength > BUFFER_SIZE){
        fread(buffer, sizeof(char), BUFFER_SIZE, stdin);
        bytesRead = BUFFER_SIZE;
    }
    else{
        fread(buffer, sizeof(char), contentLength - 9, stdin);
        bytesRead = contentLength - 9;
    }

    char *filenameStart = strstr(buffer, "filename=\"");
    if(filenameStart){
        filenameStart += 10;  // Move past "filename=\""
        char *filenameEnd = strchr(filenameStart, '\"');
        if(filenameEnd){
            strncpy(filename, filenameStart, filenameEnd - filenameStart);
            filename[filenameEnd - filenameStart] = '\0';
        }
    }
    else{
        printf("Filename not found!\n");
        return;
    }

    fileStart = strstr(buffer, "text/plain");
    if(fileStart){
        fileStart += 12;
        fileEnd = strstr(fileStart, boundary);
        fileEnd -= 2;
        if(fileEnd){
            *fileEnd = '\0';
        }
    }
    else{
        printf("File content not found!\n");
        return;
    }

    outputFile = fopen(logFilename, "a");
    if(outputFile){
        fprintf(outputFile, "\n%s\n", filename);
        fprintf(outputFile, "%s\n", fileStart);
        fclose(outputFile);

        printf("File uploaded and the log.txt has sucessfully been appended with the uploaded file's name and content!\n");
    }
    else{
        printf("Could not open the log.txt file!\n");
        return;
    }
}


