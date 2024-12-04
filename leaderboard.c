#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define MAX_NAME_LENGTH 30
#define MAX_PARTICIPANTS 1000

typedef struct{
    unsigned position;
    char name[MAX_NAME_LENGTH];
    double score;
} Participant;

int isTxtFile(char *fileName);
int writeToBuffer(FILE *file, char *buffer);
void extractNamesAndScores(char *buffer, Participant participant[], unsigned *numParticipants);
int compareParticipants(const void *a, const void *b);
void assignPositions(Participant participant[], unsigned numParticipants);

int main(int argc, char *argv[]){
    if(argc != 2){
        // must enter the name of file
        return 0;
    }

    FILE *dataFile;
    char *fileName = argv[1];

    int fileOpened = 0;
    if(isTxtFile(fileName)){
        dataFile = fopen(fileName, "r");
        if(dataFile != NULL){
            fileOpened = 1;
        }
        else{
            // could not open file!
        }
    }
    else{
        // no valid file name!
    }

    if(!fileOpened){
        // no valid file --> can't continue
        return 0;
    }

    char buffer[BUFFER_SIZE];
    unsigned numParticipants = 0;
    Participant participant[MAX_PARTICIPANTS];

    while(writeToBuffer(dataFile, buffer)){
        extractNamesAndScores(buffer, participant, &numParticipants);
    }

    fclose(dataFile);

    qsort(participant, numParticipants, sizeof(Participant), compareParticipants);
    assignPositions(participant, numParticipants);

    // printing out results for testing purposes 
    for(int i = 0; i < numParticipants; ++i){
        printf("Ranking: %u\t Name: %s\t Score: %lf\n", participant[i].position, participant[i].name, participant[i].score);
    }

    return 0;
}

int isTxtFile(char *fileName){
    int length = strlen(fileName);

    if(length < 4){
        // not a .txt file!
        return 0;
    }

    char *extension = &fileName[length - 1 - 3];
    if(strcmp(extension, ".txt") != 0){
        // not a .txt file!
        return 0;
    }

    return 1;
}

int writeToBuffer(FILE *file, char *buffer){
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
        // participant's name and score is longer than BUFFER_SIZE - 1 characters
        // if no check --> posibility of infinite loop
        // set some error code
        return 0;
    }

    return bytesRead;
}

void extractNamesAndScores(char *buffer, Participant participant[], unsigned *numParticipants){
    char *line = strtok(buffer, "\n");

    while(line != NULL){
        char name[MAX_NAME_LENGTH];
        double score;

        if(sscanf(line, "%s %lf", name, &score) == 2){
            strncpy(participant[*numParticipants].name, name, MAX_NAME_LENGTH - 1);
            participant[*numParticipants].name[MAX_NAME_LENGTH - 1] = '\0';

            participant[*numParticipants].score = score;
            ++(*numParticipants);
        }

        if(*numParticipants >= MAX_PARTICIPANTS){
            return;
        }

        line = strtok(NULL, "\n");
    }
}

int compareParticipants(const void *a, const void *b){
    const Participant *pa = (const Participant *)a;
    const Participant *pb = (const Participant *)b;

    if(pa->score < pb->score){
        return 1;
    }else if (pa->score > pb->score){
        return -1; 
    }else{
        return 0;
    }
}

void assignPositions(Participant participant[], unsigned numParticipants){
    for(int i = 0; i < numParticipants; ++i){
        participant[i].position = i + 1;
    }
}