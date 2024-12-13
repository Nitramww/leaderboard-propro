#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void generateData(char *name, char *surname, int *score) {
    const char *names[] = {
        "John", "Jane", "Alice", "Bob", "Michael", "Sarah", "David", "Laura",
        "James", "Emily", "Robert", "Emma", "William", "Olivia", "Charles", 
        "Sophia", "Thomas", "Isabella", "George", "Mia", "Henry", "Charlotte",
        "Daniel", "Amelia", "Matthew", "Harper", "Joseph", "Evelyn", "Samuel", 
        "Abigail"
    };
    const char *surnames[] = {
        "Doe", "Smith", "Johnson", "Williams", "Brown", "Jones", "Garcia", 
        "Miller", "Davis", "Rodriguez", "Martinez", "Hernandez", "Lopez", 
        "Gonzalez", "Wilson", "Anderson", "Thomas", "Taylor", "Moore", 
        "Jackson", "Martin", "Lee", "Perez", "Thompson", "White", "Harris", 
        "Sanchez", "Clark", "Ramirez", "Lewis"
    };
    
    int nameIndex = rand() % 30;
    int surnameIndex = rand() % 30;
    
    strcpy(name, names[nameIndex]);
    strcpy(surname, surnames[surnameIndex]);
    
    *score = rand() % 1001;
}
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <number_of_entries> <output_file>\n", argv[0]);
        return 1;
    }

    char *outputFileName = argv[2];
    int len = strlen(outputFileName);
    if (len < 4 || strcmp(outputFileName + len - 4, ".txt") != 0) {
        printf("The output file must have a .txt extension.\n");
        return 1;
    }

    int numEntries = atoi(argv[1]);
    if (numEntries <= 0) {
        printf("The number of entries must be a positive integer.\n");
        return 1;
    }

    srand(time(NULL));

    FILE *file = fopen(outputFileName, "w");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    char name[50];
    char surname[50];
    int score;

    for (int i = 0; i < numEntries; i++) {
        generateData(name, surname, &score);
        fprintf(file, "%s %s %d\n", name, surname, score);
    }

    fclose(file);

    printf("Data generated and written to %s\n", outputFileName);

    return 0;
}
