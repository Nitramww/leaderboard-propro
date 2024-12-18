#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_FILE_SIZE 1024
#define BUFFER_SIZE 1024

#define MAX_NAME_LENGTH 30
#define MAX_PARTICIPANTS 1000

const char tempFilename[] = "temp.txt";
const char logFilename[] = "log.txt";

typedef struct{
    unsigned position;
    char name[MAX_NAME_LENGTH];
    char surname[MAX_NAME_LENGTH];
    double score;
} Participant;

void create_temp_file_with_uploaded_data(char *boundary, int contentLength, char *inputFilename);
int is_txt_file(char *fileName);
int write_to_buffer(FILE *file, char *buffer);
void participant_extract_names_and_scores(char *buffer, Participant participant[], unsigned *participantCount);
int participant_compare(const void *a, const void *b);
void participant_assign_positions(Participant participant[], unsigned participantCount);

int main(){
    printf("Content-Type: text/html; charset=utf-8\r\n\r\n");

// ***** Processing the multipart data and writing the uploaded file's content to temp.txt *****

    char *contentType = getenv("CONTENT_TYPE");
    if(contentType == NULL){
        printf("CONTENT_TYPE not found!\n");
        return 1;
    }

    char *contentLengthStr = getenv("CONTENT_LENGTH");
    if(contentLengthStr == NULL){
        printf("CONTENT_LENGTH not found!\n");
        return 1;
    }

    int contentLength = atoi(contentLengthStr);
    char inputFilename[MAX_NAME_LENGTH] = {0};

    char *boundary = strstr(contentType, "boundary=");
    if(boundary == NULL){
        printf("\"boundary=\" not found in CONTENT_TYPE!\n");
        return 1;
    }
    
    boundary += 9;  // move past "boundary="

    if(contentLength <= MAX_INPUT_FILE_SIZE){
        create_temp_file_with_uploaded_data(boundary, contentLength, inputFilename);
    }
    else{
        printf("The uploaded input file exceeds the maximum file size of %d bytes!\n", MAX_INPUT_FILE_SIZE);
        return 1;
    }

// ***** Reading the data from temp.txt and extracting participant data ***** 

    char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
    Participant *participant = (Participant *)malloc(sizeof(Participant) * MAX_PARTICIPANTS);
    unsigned participantCount = 0;

    FILE *tempFile = fopen(tempFilename, "r");
    if(tempFile == NULL){
        printf("Could not open %s!\n", tempFilename);
        return 1;
    }

    while(write_to_buffer(tempFile, buffer)){
        participant_extract_names_and_scores(buffer, participant, &participantCount);
    }

    fclose(tempFile);

// ***** Sorting the participants' data, assigning positions and appending the results to log.txt *****

    qsort(participant, participantCount, sizeof(Participant), participant_compare);
    participant_assign_positions(participant, participantCount);

    FILE *logFile = fopen(logFilename, "a");
    if(logFile == NULL){
        printf("Could not open %s!\n", logFile);
    }

    fprintf(logFile, "%s\n", inputFilename);
    for(int i = 0; i < participantCount; ++i){
        fprintf(logFile, "%3u. %-20s %-20s %10.2lf\n", participant[i].position, participant[i].name, participant[i].surname, participant[i].score);
    }
    fprintf(logFile, "\n");
    
    fclose(logFile);
    free(participant);
    free(buffer);

    printf("check da log file.\n");

    return 0;
}

void create_temp_file_with_uploaded_data(char *boundary, int contentLength, char *inputFilename){
    char buffer[BUFFER_SIZE] = {0};
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
            strncpy(inputFilename, filenameStart, filenameEnd - filenameStart);
            inputFilename[filenameEnd - filenameStart] = '\0';
        }
    }
    else{
        printf("Filename not found!\n");
        return;
    }

    fileStart = strstr(buffer, "text/plain");
    if(fileStart){
        fileStart += 12;    // move to the beginning of file's content
        fileEnd = strstr(fileStart, boundary);
        fileEnd -= 2;       // move to the end of file's content
        if(fileEnd){
            *fileEnd = '\0';
        }
    }
    else{
        printf("File content not found!\n");
        return;
    }

    outputFile = fopen(tempFilename, "w");
    if(outputFile){
        fprintf(outputFile, "%s\n", fileStart);
        fclose(outputFile);

        printf("File uploaded and temp.txt has sucessfully been appended with the uploaded file's content!\n");
    }
    else{
        printf("Could not open temp.txt!\n");
        return;
    }
}

int is_txt_file(char *fileName){
    int length = strlen(fileName);

    if(length < 4){
        return 0;
    }

    char *extension = &fileName[length - 4];
    if(strcmp(extension, ".txt") != 0){
        return 0;
    }

    return 1;
}

int write_to_buffer(FILE *file, char *buffer){
    unsigned lastPos = ftell(file);
    int bytesRead = fread(buffer, sizeof(char), BUFFER_SIZE - 1, file);

    if(bytesRead == 0){
        return 0;
    }

    // ensuring the buffer ends at '\n' or EOF
    if(bytesRead == BUFFER_SIZE - 1){
        int backtrackIndex = bytesRead - 1;

        while(backtrackIndex >= 0 && buffer[backtrackIndex] != '\n'){
            --backtrackIndex;
        }

        if(backtrackIndex >= 0){
            fseek(file, backtrackIndex - bytesRead + 1, SEEK_CUR);
            buffer[backtrackIndex] = '\0';
        }
    }
    else{
        buffer[bytesRead] = '\0';
    }

    unsigned currentPos = ftell(file);
    if(lastPos == currentPos){
        return 0;
    }

    return bytesRead;
}

void participant_extract_names_and_scores(char *buffer, Participant participant[], unsigned *participantCount){
    char *line = strtok(buffer, "\n");

    while(line != NULL){
        char name[MAX_NAME_LENGTH];
        char surname[MAX_NAME_LENGTH];
        double score;

        if(sscanf(line, "%s%s%lf", name, surname, &score) == 3){
            strncpy(participant[*participantCount].name, name, MAX_NAME_LENGTH - 1);
            participant[*participantCount].name[MAX_NAME_LENGTH - 1] = '\0';
            strncpy(participant[*participantCount].surname, surname, MAX_NAME_LENGTH - 1);
            participant[*participantCount].score = score;
            ++(*participantCount);
        }

        if(*participantCount >= MAX_PARTICIPANTS){
            return;
        }

        line = strtok(NULL, "\n");
    }
}

int participant_compare(const void *a, const void *b){
    const Participant *pa = (const Participant *)a;
    const Participant *pb = (const Participant *)b;

    if (pa->score < pb->score){
        return 1;
    } else if (pa->score > pb->score){
        return -1; 
    } else{
        return 0;
    }
}

void participant_assign_positions(Participant participant[], unsigned participantCount){
    for(int i = 0; i < participantCount; ++i){
        participant[i].position = i + 1;
    }
}

