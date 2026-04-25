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

//PHASE 1
void add(char *downtown, char *role, char *username);
void list(char *downtown, char *role, char *username);
void view(char *downtown, char *id, char *role, char *username);
void remove_report(char *downtown, char *id, char *role, char *username);
void update_threshold(char *downtown, char *value, char *role, char *username);


#endif //PROIECT_FUNCTIONS_H
