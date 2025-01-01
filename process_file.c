#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_FILE_SIZE 1024
#define BUFFER_SIZE 1024

#define MAX_NAME_LENGTH 30
#define MAX_PARTICIPANTS 100

const char tempFilename[] = "temp.txt";
const char logFilename[] = "log.txt";
const char htmlFilename[] = "../index.html";

typedef struct{
    unsigned position;
    char name[MAX_NAME_LENGTH];
    char surname[MAX_NAME_LENGTH];
    double score;
} Participant;

void temp_file_create_with_uploaded_data(char *boundary, int contentLength, char *inputFilename);
int  is_txt_file(char *fileName);
int  write_to_buffer(FILE *file, char *buffer);

void participant_extract_names_and_scores(char *buffer, Participant participant[], unsigned *participantCount);
int  participant_compare(const void *a, const void *b);
void participant_assign_positions(Participant participant[], unsigned participantCount);

void html_write_before_data(FILE *htmlFile);
void html_write_data(FILE *logFile, FILE *htmlFile);
void html_write_after_data(FILE *htmlFile);
void html_write_event_name(FILE *htmlFile, char *eventFilename, int id);
void html_write_participant(FILE *htmlFile, char *pos, char *name, char *surname, char *score);
void html_write_end_of_previous_event(FILE *htmlFile);
void html_write_with_all_data(FILE *logFile, FILE *htmlFile);

int main(){
    printf("Content-Type: text/html; charset=utf-8\r\n\r\n");

// ***** Processing the multipart data and writing the uploaded file's content to temp.txt *****

    char *contentType = getenv("CONTENT_TYPE");

    if(contentType == NULL){
        printf("Error: CONTENT_TYPE not found!\n");
        return 1;
    }

    char *contentLengthStr = getenv("CONTENT_LENGTH");

    if(contentLengthStr == NULL){
        printf("Error: CONTENT_LENGTH not found!\n");
        return 1;
    }

    int contentLength = atoi(contentLengthStr);
    char inputFilename[MAX_NAME_LENGTH] = {0};

    char *boundary = strstr(contentType, "boundary=");

    if(boundary == NULL){
        printf("Error: \"boundary=\" not found in CONTENT_TYPE!\n");
        return 1;
    }

    if(contentLength > MAX_INPUT_FILE_SIZE){
        printf("Error: The uploaded input file exceeds the maximum file size of %d bytes!\n", MAX_INPUT_FILE_SIZE);
        return 1;
    }

    boundary += 9;  // move past "boundary="
    temp_file_create_with_uploaded_data(boundary, contentLength, inputFilename);

// ***** Reading the data from temp.txt and extracting participant data ***** 

    char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
    Participant *participant = (Participant *)malloc(sizeof(Participant) * MAX_PARTICIPANTS);
    unsigned participantCount = 0;

    FILE *tempFile = fopen(tempFilename, "r");

    if(tempFile == NULL){
        printf("Error: Could not open %s!\n", tempFilename);
        return 1;
    }

    while(write_to_buffer(tempFile, buffer)){
        participant_extract_names_and_scores(buffer, participant, &participantCount);
    }

    fclose(tempFile);

    if(participantCount > MAX_PARTICIPANTS){
        printf("Error: Maximum participant limit exceeded, the count must not be over %d!\n", MAX_PARTICIPANTS);
        return 1;
    }

// ***** Sorting the participants' data, assigning positions and appending the results to log.txt *****

    qsort(participant, participantCount, sizeof(Participant), participant_compare);
    participant_assign_positions(participant, participantCount);

    FILE *logFile = fopen(logFilename, "a");

    if(logFile == NULL){
        printf("Error: Could not open %s!\n", logFilename);
        free(participant);
        free(buffer);
        return 1;
    }

    fprintf(logFile, "%s\n", inputFilename);
    for(int i = 0; i < participantCount; ++i){
        fprintf(logFile, "%3u %-20s %-20s %10.2lf\n", participant[i].position, participant[i].name, participant[i].surname, participant[i].score);
    }
    fprintf(logFile, "\n");
    
    fclose(logFile);
    free(participant);
    free(buffer);

// ***** Reading data from log.txt and creating an .html with all event's data *****

    FILE *htmlFile = fopen(htmlFilename, "w");

    if(htmlFile == NULL){
        printf("Error: Could not open %s!\n", htmlFilename);
        return 1;
    }

    logFile = fopen(logFilename, "r");

    if(logFile == NULL){
        printf("Error: Could not open %s!\n", logFilename);
        return 1;
    }    

    html_write_with_all_data(logFile, htmlFile);

    fclose(htmlFile);
    fclose(logFile);

    printf("A new leaderboard has successfuly been created! Please navigate back to main page.\n");

    return 0;
}

void temp_file_create_with_uploaded_data(char *boundary, int contentLength, char *inputFilename){
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
        printf("Error: Filename not found!\n");
        return;
    }

    fileStart = strstr(buffer, "text/plain");

    if(fileStart == NULL){
        printf("Error: File content not found!\n");
        return;
    }

    fileStart += 12;    // move to the beginning of file's content
    fileEnd = strstr(fileStart, boundary);
    fileEnd -= 2;       // move to the end of file's content
    if(fileEnd){
        *fileEnd = '\0';
    }

    outputFile = fopen(tempFilename, "w");

    if(outputFile == NULL){
        printf("Error: Could not open temp.txt!\n");
        return;
    }

    fprintf(outputFile, "%s\n", fileStart);
    fclose(outputFile);
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

void html_write_before_data(FILE *htmlFile){
    fprintf(htmlFile, 
      "<!DOCTYPE html>\n"
      "<html lang=\"en\">\n"
      "<head>\n"
      "    <meta charset=\"UTF-8\">\n"
      "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
      "    <title>Leaderboard</title>\n"
      "    <link rel=\"stylesheet\" href=\"styles.css\">\n"
      "    <link href=\"https://fonts.googleapis.com/css2?family=Noto+Serif:ital,wght@0,100..900;1,100..900&display=swap\" rel=\"stylesheet\">\n"
      "</head>\n"
      "<body>\n"
      "    <header>\n"
      "        <h1>Leaderboards</h1>\n"
      "        <p class=\"subtitle\">Sort scores and keep track of past events</p>\n"
      "    </header>\n"
      "    <nav>\n"
      "      <a href=\"./index.html\">Home</a>\n"
      "      <a href=\"./about.html\">About</a>\n"
      "    </nav>\n"
      "    <button popovertarget=\"popup\" class=\"add-event-btn\">How to use?</button>\n"
      "    <div id=\"popup\" popover>\n"
      "        <h2>Upload your tournament</h2>\n"
      "        <p></b>The name of the .txt file is interpreted as the tournament name!<br>The data does not have to be sorted by score</b></p>\n"
      "        <p>Make sure it is in the format of:</p>\n"
      "        <p>Name Surname Score<br>Name Surname Score</p>\n"
      "    </div>\n"
      "    <section class=\"accordion\">\n"
      "        <form action=\"/cgi-bin/process_file.exe\" class=\"upload_file\" method=\"post\" enctype=\"multipart/form-data\">\n"
      "            <label for=\"file\">Select a .txt file:</label>\n"
      "            <input type=\"file\" id=\"file\" name=\"file\" accept=\".txt\" required>\n"
      "            <button type=\"submit\" class=\"upload-btn\">Upload</button>\n"
      "        </form>\n"
    );
}

void html_write_data(FILE *logFile, FILE *htmlFile){
    char *buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
    unsigned id = 1;

    int bytesRead;
    while((bytesRead = write_to_buffer(logFile, buffer)) > 0){
        char *token = strtok(buffer, " \t\n");

        while(token != NULL){
            if(is_txt_file(token)){
                if(id != 1){
                    html_write_end_of_previous_event(htmlFile);
                }

                html_write_event_name(htmlFile, token, id);

                ++id;
                token = strtok(NULL, " \t\n");
            }
            else{
                char *pos = token;
                char *name = strtok(NULL, " \t");
                char *surname = strtok(NULL, " \t");
                char *score = strtok(NULL, "\n");

                html_write_participant(htmlFile, pos, name, surname, score);

                token = strtok(NULL, " \t\n");
            }
        }
    }

    html_write_end_of_previous_event(htmlFile);

    free(buffer);
}

void html_write_event_name(FILE *htmlFile, char *eventFilename, int id){
    fprintf(htmlFile, 
      "        <div class=\"tab\">"
      "          <input type=\"checkbox\" name=\"accordion-1\" id=\"%d\">\n"
      "          <label for=\"%d\" class=\"tab__label\">%s</label>\n"
      "          <div class=\"tab__content\">\n"
      "            <div class=\"participants\">\n",
      id, id, eventFilename
    );
}

void html_write_participant(FILE *htmlFile, char *pos, char *name, char *surname, char *score){
    fprintf(htmlFile, 
      "                  <p><span class=\"position\">#%s"
      "</span><span class=\"name\">%s %s"
      "</span><span class=\"score\">%s</span></p>\n",
      pos, name, surname, score
    );
}

void html_write_end_of_previous_event(FILE *htmlFile){
    fprintf(htmlFile, 
      "            </div>\n"
      "          </div>\n"
      "        </div>\n"
    );
}

void html_write_after_data(FILE *htmlFile){
    fprintf(htmlFile, 
      "      </section>\n"
      "  </body>\n"
      "</html>\n"
    );
}

void html_write_with_all_data(FILE *logFile, FILE *htmlFile){
    html_write_before_data(htmlFile);
    html_write_data(logFile, htmlFile);
    html_write_after_data(htmlFile);
}

