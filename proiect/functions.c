//
// Created by astef on 04/21/2026.
//

#include "functions.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

void set_role(char *choosen_role, char *role) {
    strcpy(role, choosen_role);
}

void set_username(char *choosen_username, char *username) {
    strcpy(username, choosen_username);
}

void config_reports_file(int file, char *username, char *report_path) {
    report_t report;

    // contorizam cate rapoarte avem si creeam un nou id - bazat pe contor
    struct stat st;
    stat(report_path, &st);
    int num_reports = st.st_size / sizeof(report_t);
    report.report_id = num_reports + 1;

    strcpy(report.inspector_name, username);

    // citire date
    printf("Please enter de coordinates\n");
    printf("Longitude:");
    scanf("%f", &report.gps_coordinates.longitude);
    printf("Latitude:");
    scanf("%f", &report.gps_coordinates.latitude);

    printf("Please enter category (road/lighting/flooding/etc.):");
    scanf("%s", report.issue_category);

    printf("Please introduce severity level (1/2/3):");
    scanf("%d", &report.severity_level);

    report.timestamp = time(NULL);

    printf("Please enter a description:");
    scanf("%s", report.description);

    write(file, &report, sizeof(report_t));
}

void config_logged_district(int file, char *role, char *username, char *function) {
    // extragere data curenta
    time_t timestamp = time(NULL);

    // stabilire format + scriere in fisier
    char log[256];
    snprintf(log, sizeof(log), "[%ld] role=%s user=%s action=%s\n",(long) timestamp, role, username, function);
    write(file, log, strlen(log));
}

void add(char *district_id, char *role, char *username) {
    // creare path: Districts/<Nume-district>
    char path[128];
    strcpy(path, "Districts/");
    strcat(path, district_id);

    // creare folder district
    mkdir(path, 0750);
    chmod(path, 0750);

    char report_path[256]; // report_path o sa retina pe rand path-ul catre fiecare file

    // creare / append in reports.dat
    snprintf(report_path, 256, "%s/%s", path, "reports.dat");
    int fd1 = open(report_path, O_RDWR | O_CREAT | O_APPEND, 0644);
    chmod(report_path, 0664);

    config_reports_file(fd1, username, report_path); // scriere in fisier

    close(fd1);

    // creare / append in district.cfg
    snprintf(report_path, 256, "%s/%s", path, "district.cfg");
    int fd2 = open(report_path, O_RDWR | O_CREAT, 0640);
    chmod(report_path, 0640);
    close(fd2);

    // creare / append logged_district
    snprintf(report_path, 256, "%s/%s", path, "logged_district");
    int fd3 = open(report_path, O_RDWR | O_CREAT | O_APPEND, 0644);
    chmod(report_path, 0644);

    config_logged_district(fd3, role, username, "add"); // scriere in logged_district

    close(fd3);

    // creare symlink
    char symlink_path[128];
    char target_path[256];

    snprintf(symlink_path, sizeof(symlink_path), "active_reports-%s", district_id);
    snprintf(target_path, sizeof(target_path), "%s/reports.dat", path);

    symlink(target_path, symlink_path);
}
