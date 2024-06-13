#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_LINE_LENGTH 1024
#define MAX_TRADE_OBJECTS 1024
#define CURRENCY_CODE_LENGTH 3

typedef struct {
    char sourceCurrency[CURRENCY_CODE_LENGTH + 1]; // SourceCurrency
    char destibationCurrency[CURRENCY_CODE_LENGTH + 1]; // DestinationCurrency
    int Lots;
    double Price;
} TradeRecord;

char** SplitString(const char* str, char delimiter) {
    int count = 0;
    const char* ptr = str;
    while (*ptr != '\0') {
        if (*ptr++ == delimiter) {
            count++;
        }
    }

    char** tokens = malloc(sizeof(char*) * (count + 2));
    if (!tokens) {
        fprintf(stderr, "ERROR: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    ptr = str;
    char* token = malloc(strlen(str) + 1);
    if (!token) {
        fprintf(stderr, "ERROR: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    int j = 0;
    while (*ptr != '\0') {
        if (*ptr == delimiter) {
            token[j] = '\0';
            tokens[i++] = strdup(token);
            if (!tokens[i - 1]) {
                fprintf(stderr, "ERROR: Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
            j = 0;
        } else {
            token[j++] = *ptr;
        }
        ptr++;
    }
    token[j] = '\0';
    tokens[i++] = strdup(token);
    if (!tokens[i - 1]) {
        fprintf(stderr, "ERROR: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    tokens[i] = NULL;
    free(token);
    return tokens;
}

int parseIntFromString(const char* str, int* value) {
    char* endptr;
    errno = 0; // Reset errno before calling strtol
    long parsed = strtol(str, &endptr, 10);
    if (errno != 0 || endptr == str) {
        return 0; // Failed to parse or no digits found
    }
    *value = (int)parsed;
    return 1; // Successfully parsed
}

int parseDouble(const char* str, double* value) {
    char* endptr;
    errno = 0; // Reset errno before calling strtod
    *value = strtod(str, &endptr);
    if (errno != 0 || endptr == str) {
        return 0; // Failed to parse or no valid double found
    }
    return 1; // Successfully parsed
}

void Process(FILE* stream) {
    char line[MAX_LINE_LENGTH];
    TradeRecord objects[MAX_TRADE_OBJECTS];
    int lineCount = 0;
    int objectCount = 0;

    while (fgets(line, sizeof(line), stream)) {
        char* fields[3];
        int fieldCount = 0;
        char* token = strtok(line, ",");
        while (token != NULL) {
            fields[fieldCount++] = token;
            token = strtok(NULL, ",");
        }

        if (fieldCount != 3) {
            fprintf(stderr, "WARN: Line %d malformed. Only %d field(s) found.\n", lineCount + 1, fieldCount);
            continue;
        }

        if (strlen(fields[0]) != CURRENCY_CODE_LENGTH * 2) {
            fprintf(stderr, "WARN: Trade currencies on line %d malformed: '%s'\n", lineCount + 1, fields[0]);
            continue;
        }

        int tradeAmount;
        if (!parseIntFromString(fields[1], &tradeAmount)) {
            fprintf(stderr, "WARN: Trade amount on line %d not a valid integer: '%s'\n", lineCount + 1, fields[1]);
            continue;
        }

        double tradePrice;
        if (!parseDouble(fields[2], &tradePrice)) {
            fprintf(stderr, "WARN: Trade price on line %d not a valid decimal: '%s'\n", lineCount + 1, fields[2]);
            continue;
        }

        strncpy(objects[objectCount].sourceCurrency, fields[0], CURRENCY_CODE_LENGTH);
        strncpy(objects[objectCount].destibationCurrency, fields[0] + CURRENCY_CODE_LENGTH, CURRENCY_CODE_LENGTH);
        objects[objectCount].Lots = tradeAmount / LotSize; // Assuming LotSize is defined elsewhere
        objects[objectCount].Price = tradePrice;
        objectCount++;
        lineCount++;
    }

    FILE* outFile = fopen("output.xml", "w");
    if (!outFile) {
        fprintf(stderr, "ERROR: Unable to open output file for writing\n");
        exit(EXIT_FAILURE);
    }
    fprintf(outFile, "<TradeRecords>\n");
    for (int i = 0; i < objectCount; i++) {
        fprintf(outFile, "\t<TradeRecord>\n");
        fprintf(outFile, "\t\t<SourceCurrency>%s</SourceCurrency>\n", objects[i].sourceCurrency);
        fprintf(outFile, "\t\t<DestinationCurrency>%s</DestinationCurrency>\n", objects[i].destibationCurrency);
        fprintf(outFile, "\t\t<Lots>%d</Lots>\n", objects[i].Lots);
        fprintf(outFile, "\t\t<Price>%f</Price>\n", objects[i].Price);
        fprintf(outFile, "\t</TradeRecord>\n");
    }
    fprintf(outFile, "</TradeRecords>");
    fclose(outFile);
    printf("INFO: %d trades processed\n", objectCount);
}
