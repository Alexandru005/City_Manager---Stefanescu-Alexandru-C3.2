//
// Created by astef on 04/21/2026.
//

#ifndef PROIECT_FUNCTIONS_H
#define PROIECT_FUNCTIONS_H

#include <time.h>

typedef struct gps {
    float latitude;
    float longitude;
}gps_t;

typedef struct Report {
    int report_id;
    char inspector_name[50];
    gps_t gps_coordinates;
    char issue_category[25];
    int severity_level;
    time_t timestamp;
    char description[128];
}report_t;

void set_role(char *choosen_role, char *role);
void set_username(char *choosen_username, char *username);
void add(char *downtown, char *role, char *username);

#endif //PROIECT_FUNCTIONS_H
