#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "functions.h" // Pentru a recunoaște structura report_t

// Structură auxiliară pentru a salva scorurile temporar
typedef struct {
    char inspector[50];
    int score;
} InspectorScore;

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Nu a fost furnizat un district!\n");
        return 1;
    }

    char *district = argv[1];
    char path[128];
    snprintf(path, sizeof(path), "Districts/%s/reports.dat", district);

    int file = open(path, O_RDONLY);
    if (file < 0) {
        printf("Nu s-au gasit rapoarte sau districtul nu exista (%s).\n", district);
        return 1;
    }

    InspectorScore scores[100]; // Presupunem un maxim de 100 de inspectori diferiți
    int count = 0;
    report_t report;

    // Citim raport cu raport și calculăm scorul
    while (read(file, &report, sizeof(report_t)) == sizeof(report_t)) {
        int found = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(scores[i].inspector, report.inspector_name) == 0) {
                scores[i].score += report.severity_level;
                found = 1;
                break;
            }
        }
        // Dacă e un inspector nou, îl adăugăm în listă
        if (found == 0 && count < 100) {
            strcpy(scores[count].inspector, report.inspector_name);
            scores[count].score = report.severity_level;
            count++;
        }
    }

    close(file);

    // Afișăm rezultatul (va fi trimis automat prin pipe către city_hub datorită lui dup2)
    for (int i = 0; i < count; i++) {
        printf("Inspector: %-15s | Scorul total: %d\n", scores[i].inspector, scores[i].score);
    }

    return 0;
}