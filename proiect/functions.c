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
    memset(&report, 0, sizeof(report_t)); // setam fiecare byte pe 0 pt a fi mai usor de citit datele vizual

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
    do {
        scanf("%d", &report.severity_level);

        if (report.severity_level < 1 || report.severity_level > 3) {
            printf("Please chose again (1 OR 2 OR 3):");
        }

    }while (1 > report.severity_level || report.severity_level > 3);

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

void get_permission(struct stat st, char *perm) {
    if (st.st_mode & S_IRUSR) perm[0] = 'r'; else perm[0] = '-';
    if (st.st_mode & S_IWUSR) perm[1] = 'w'; else perm[1] = '-';
    if (st.st_mode & S_IXUSR) perm[2] = 'x'; else perm[2] = '-';

    if (st.st_mode & S_IRGRP) perm[3] = 'r'; else perm[3] = '-';
    if (st.st_mode & S_IWGRP) perm[4] = 'w'; else perm[4] = '-';
    if (st.st_mode & S_IXGRP) perm[5] = 'x'; else perm[5] = '-';

    if (st.st_mode & S_IROTH) perm[6] = 'r'; else perm[6] = '-';
    if (st.st_mode & S_IWOTH) perm[7] = 'w'; else perm[7] = '-';
    if (st.st_mode & S_IXOTH) perm[8] = 'x'; else perm[8] = '-';

    perm[9] = '\0';
}

void list(char *downtown, char *role, char *username) {
    // path directory
    char path[64];
    snprintf(path, sizeof(path), "%s/%s", "Districts", downtown);

    // verificam ca downtown - ul introdus sa existe
    struct stat st;
    if (stat(path, &st) != 0) {
        printf("District '%s' does not exist!\n", downtown);
        return;
    }

    char report_path[128];
    snprintf(report_path, sizeof(report_path), "%s/%s", path, "reports.dat");


    int report_file = open(report_path, O_RDONLY);

    // afisare
    report_t report;

    while (read(report_file, &report, sizeof(report_t)) == sizeof(report_t)) {
        printf("ID: %d\n", report.report_id);
        printf("User: %s\n", report.inspector_name);
        printf("Category: %s\n", report.issue_category);
        printf("Severity: %d\n", report.severity_level);
        printf("GPS: (%.2f, %.2f)\n", report.gps_coordinates.latitude, report.gps_coordinates.longitude);
        printf("Description: %s\n", report.description);

        char time_str[64];
        struct tm *tm_info = localtime(&report.timestamp);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        printf("Timestamp: %s\n", time_str);
        printf("---------------------------------------\n\n");
    }

    stat(report_path, &st);

    printf("records.dat file details:\n");
    printf("Size: %ld bytes\n", st.st_size);

    char time_str[64];
    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("Last modified: %s\n", time_str);

    char permissions[15];
    get_permission(st, permissions);

    printf("Permissions: %s\n", permissions);

    close(report_file);

    char logged_district_path[128];
    snprintf(logged_district_path, sizeof(logged_district_path), "%s/%s", path, "logged_district");

    int logged_district_file = open(logged_district_path, O_RDWR | O_APPEND);

    config_logged_district(logged_district_file, role, username, "list");

    close(logged_district_file);
}
