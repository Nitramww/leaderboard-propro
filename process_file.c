#include <stdio.h>
#include <stdlib.h>

// this program prints the post request data to web
int main() {
    printf("Content-Type: text/html; charset=utf-8\r\n\r\n");

    char *contentType = getenv("CONTENT_TYPE");
    printf("content type: %s<br>", contentType ? contentType : "Not provided");

    char *contentLength = getenv("CONTENT_LENGTH");
    printf("content length: %s<br>", contentLength ? contentLength : "Not provided");

    int length = 0;
    char *buffer = NULL;

    if(contentLength){
        length = atoi(contentLength);
        if(length > 0){
            buffer = (char *)malloc(length + 1);
            if(buffer == NULL){
                printf("Memory allocation failed<br>");
                return 1;
            }

            if(fread(buffer, sizeof(char), length - 9, stdin) != length - 9){
                printf("Failed to read POST data<br>");
                free(buffer);
                return 1;
            }

            buffer[length - 9] = '\0';
            printf("<p><b>%s</b></p>", buffer);
            free(buffer);
        }
		else{
            printf("Invalid CONTENT_LENGTH<br>");
        }
    }
	else{
        printf("No CONTENT_LENGTH provided<br>");
    }

    return 0;
}
