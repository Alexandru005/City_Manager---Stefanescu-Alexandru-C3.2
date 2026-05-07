#include "functions.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

// set the role for the user given in the command line
void set_role(char *choosen_role, char *role) {
    strcpy(role, choosen_role);
}

// set the username for the user given in the command line
void set_username(char *choosen_username, char *username) {
    strcpy(username, choosen_username);
}

// read & write the data for the reports.dat file
void config_reports_file(int file, char *username, char *report_path) {
    report_t report;
    memset(&report, 0, sizeof(report_t)); // setam fiecare byte pe 0 pt a fi mai usor de citit datele vizual

    // contorizam cate rapoarte avem si creeam un nou id - bazat pe contor
    struct stat st;
    stat(report_path, &st);
    int num_reports = st.st_size / sizeof(report_t);

    int max_id = 0;
    report_t report_aux;
    for (int i = 0; i < num_reports; i++) {
        read(file, &report_aux, sizeof(report_t));
        if (report_aux.report_id > max_id) {
            max_id = report_aux.report_id;
        }
    }

    report.report_id = max_id + 1;

    strcpy(report.inspector_name, username);

    // citire date
    printf("Please enter de coordinates\n");
    printf("Longitude:");
    scanf("%f", &report.gps_coordinates.longitude);
    printf("Latitude:");
    scanf("%f", &report.gps_coordinates.latitude);

    printf("Please enter category (road/lighting/flooding/etc.):");
    scanf("%24s", report.issue_category);

    printf("Please introduce severity level (1/2/3):");
    do {
        scanf("%d", &report.severity_level);

        if (report.severity_level < 1 || report.severity_level > 3) {
            printf("Please chose again (1 OR 2 OR 3):");
        }

    }while (1 > report.severity_level || report.severity_level > 3);


    printf("Please enter a description:");
    scanf(" %127[^\n]", report.description);

    report.timestamp = time(NULL);

    write(file, &report, sizeof(report_t));
}

// read & write the data for the config_logged file
void config_logged_district(int file, char *role, char *username, char *function) {
    // extragere data curenta
    time_t timestamp = time(NULL);

    // stabilire format + scriere in fisier
    char log[256];
    snprintf(log, sizeof(log), "[%ld] role=%s user=%s action=%s\n",(long) timestamp, role, username, function);
    write(file, log, strlen(log));
}

// implementation for function add
// creates directory
// creates all files
// initialize reports.dat & logged_districts files
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
    int fd1 = open(report_path, O_RDWR | O_CREAT | O_APPEND, 0664);
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

    // notificare monitor
    int monitor_fd = open(".monitor_pid", O_RDONLY);
    if (monitor_fd == -1) {
        // fisierul nu exista - redeschidem logul si scriem
        int fd_log = open(report_path, O_RDWR | O_APPEND);
        config_logged_district(fd_log, role, username, "add - monitor could not be notified");
        close(fd_log);
    } else {
        char pid_str[32];
        memset(pid_str, 0, sizeof(pid_str));
        read(monitor_fd, pid_str, sizeof(pid_str));
        close(monitor_fd);

        pid_t monitor_pid = atoi(pid_str);

        int fd_log = open(report_path, O_RDWR | O_APPEND);
        if (kill(monitor_pid, SIGUSR1) == 0) {
            config_logged_district(fd_log, role, username, "add - monitor notified");
        } else {
            config_logged_district(fd_log, role, username, "add - monitor could not be notified");
        }
        close(fd_log);
    }

    // creare symlink
    char symlink_path[128];
    char target_path[256];

    snprintf(symlink_path, sizeof(symlink_path), "active_reports-%s", district_id);
    snprintf(target_path, sizeof(target_path), "%s/reports.dat", path);

    symlink(target_path, symlink_path);
}

// stores in a char variable all the permissions for each type "user"
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

//display report (only one)
void print_report(report_t *report) {
    printf("ID: %d\n", report->report_id);
    printf("User: %s\n", report->inspector_name);
    printf("Category: %s\n", report->issue_category);
    printf("Severity: %d\n", report->severity_level);
    printf("GPS: (%.2f, %.2f)\n", report->gps_coordinates.latitude, report->gps_coordinates.longitude);
    printf("Description: %s\n", report->description);

    char time_str[64];
    struct tm *tm_info = localtime(&report->timestamp);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("Timestamp: %s\n", time_str);
}

// implementation for function list
// first we verify if the district it is registered
// opens reports.dat file and list every report it has then list file details (capcity, last update, permissions)
// opens logged_district and append last operation (list)
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
        print_report(&report);
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

    // scriere in logged_district
    char logged_district_path[128];
    snprintf(logged_district_path, sizeof(logged_district_path), "%s/%s", path, "logged_district");

    int logged_district_file = open(logged_district_path, O_RDWR | O_APPEND);

    config_logged_district(logged_district_file, role, username, "list");

    close(logged_district_file);
}

// display a specific report (by id)
void view(char *downtown, char *id, char *role, char *username) {
    char path[64];
    snprintf(path, sizeof(path), "%s/%s", "Districts", downtown);

    char path_file_reports[80];
    snprintf(path_file_reports, sizeof(path_file_reports), "%s/%s", path, "reports.dat");


    int file = open(path_file_reports, O_RDONLY);

    report_t report;
    int aux_id = atoi(id);
    int found = 0;

    // cautam report - ul dupa id
    while (read(file, &report, sizeof(report_t)) == sizeof(report_t)) {
        if (report.report_id == aux_id) {
            print_report(&report);
            found = 1;
        }
    }

    close(file);

    // daca nu l-am gasit afisam mesaj si ne oprim
    if (found == 0) {
        printf("The id is not valid!\n");
        return;
    }


    // afisam in logged_district
    char path_file_logged_district[80];
    snprintf(path_file_logged_district, sizeof(path_file_logged_district), "%s/%s", path, "logged_district");

    file = open(path_file_logged_district, O_APPEND | O_RDWR);

    config_logged_district(file, role, username, "view");

    close(file);

}

// remove a report by id & verify the role of the user to be manager
void remove_report(char *downtown, char *id, char *role, char *username) {
    if (strcmp(role, "manager") == 0) { // verificam sa fie manager
        char path[64];
        snprintf(path, sizeof(path), "%s/%s", "Districts", downtown);

        char path_file_reports[80];
        snprintf(path_file_reports, sizeof(path_file_reports), "%s/%s", path, "reports.dat");

        int file = open(path_file_reports, O_RDWR);

        int indx_id_deleted = -1;
        int nr_of_reports = 0;
        int id_value = atoi(id);
        report_t report;

        // contorizam cate rapoarte avem & cautam index-ul raportului cautat
        while (read(file, &report, sizeof(report_t)) == sizeof(report_t)) {
            nr_of_reports++;
            if (report.report_id == id_value) {
                indx_id_deleted = nr_of_reports;
            }
        }

        // in caz ca nu exista afisam mesaj & ne oprim
        if (indx_id_deleted == -1) {
            printf("The id is not valid!\n");
            close(file);
            return;
        }

        // suprascriem toate rapoartele peste cele precedente incepand cu
        for (int i = indx_id_deleted; i < nr_of_reports; i++) {
            lseek(file, i * sizeof(report_t), SEEK_SET);
            read(file, &report, sizeof(report_t));

            lseek(file, (i - 1) * sizeof(report_t), SEEK_SET);
            write(file, &report, sizeof(report_t));
        }

        ftruncate(file, (nr_of_reports - 1) * sizeof(report_t));

        close(file);

        char path_file_logged_district[80];
        snprintf(path_file_logged_district, sizeof(path_file_logged_district), "%s/%s", path, "logged_district");

        file = open(path_file_logged_district, O_APPEND | O_RDWR);

        config_logged_district(file, role, username, "remove_report");

        close(file);

        printf("The report was deleted succesfully!\n");

    }else if (strcmp(role, "manager") != 0) {
        printf("You don't have the permission\n");
    }
}

// update district.cfg
void update_threshold(char *downtown, char *value, char *role, char *username) {
    if (strcmp(role, "manager") == 0) {
        char path[64];
        snprintf(path, sizeof(path), "%s/%s", "Districts", downtown);

        char path_file_threshold[80];
        snprintf(path_file_threshold, sizeof(path_file_threshold), "%s/%s", path, "district.cfg");

        int file = open(path_file_threshold, O_RDWR);

        // verify file permissions
        struct stat st;
        stat(path_file_threshold, &st);

        mode_t perms = st.st_mode & 0777;
        if (perms != 0640) {
            printf("The file permission are not 640!\n");
            return;
        }

        // update the file
        char text[128];
        snprintf(text, sizeof(text), "%s=%d\n", "threshold", atoi(value));

        write(file, text, strlen(text));

        close(file);

        char path_file_logged_district[80];
        snprintf(path_file_logged_district, sizeof(path_file_logged_district), "%s/%s", path, "logged_district");

        file = open(path_file_logged_district, O_APPEND | O_RDWR);

        config_logged_district(file, role, username, "update_threshold");

        close(file);

        printf("The threshold was updated succesfully!\n");

    }else {
        printf("You don't have the permission!\n");
    }
}

int parse_condition(const char *input, char *field, char *op, char *value) {
    // copiem input-ul ca sa nu il modificam
    char copy[256];
    strncpy(copy, input, sizeof(copy));

    // splitam dupa ':'
    char *token = strtok(copy, ":");
    if (token == NULL) return 0;
    strcpy(field, token);

    token = strtok(NULL, ":");
    if (token == NULL) return 0;
    strcpy(op, token);

    token = strtok(NULL, ":");
    if (token == NULL) return 0;
    strcpy(value, token);

    return 1;
}

int match_condition(report_t *r, const char *field, const char *op, const char *value) {
    if (strcmp(field, "severity") == 0) {
        int val = atoi(value);
        if (strcmp(op, "==") == 0) return r->severity_level == val;
        if (strcmp(op, "!=") == 0) return r->severity_level != val;
        if (strcmp(op, "<")  == 0) return r->severity_level <  val;
        if (strcmp(op, "<=") == 0) return r->severity_level <= val;
        if (strcmp(op, ">")  == 0) return r->severity_level >  val;
        if (strcmp(op, ">=") == 0) return r->severity_level >= val;
    }
    if (strcmp(field, "category") == 0) {
        int cmp = strcmp(r->issue_category, value);
        if (strcmp(op, "==") == 0) return cmp == 0;
        if (strcmp(op, "!=") == 0) return cmp != 0;
    }
    if (strcmp(field, "inspector") == 0) {
        int cmp = strcmp(r->inspector_name, value);
        if (strcmp(op, "==") == 0) return cmp == 0;
        if (strcmp(op, "!=") == 0) return cmp != 0;
    }
    if (strcmp(field, "timestamp") == 0) {
        time_t val = (time_t)atol(value);
        if (strcmp(op, "==") == 0) return r->timestamp == val;
        if (strcmp(op, "!=") == 0) return r->timestamp != val;
        if (strcmp(op, "<")  == 0) return r->timestamp <  val;
        if (strcmp(op, "<=") == 0) return r->timestamp <= val;
        if (strcmp(op, ">")  == 0) return r->timestamp >  val;
        if (strcmp(op, ">=") == 0) return r->timestamp >= val;
    }
    return 0;
}

void filter(char *downtown, char *role, char *username, int argc, char **conditions, int nr_conditions) {
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
        int all_match = 1;
        for (int i = 0; i < nr_conditions; i++) {
            char field[32], op[8], value[64];
            parse_condition(conditions[i], field, op, value);
            if (match_condition(&report, field, op, value) == 0) {
                all_match = 0;
                break;
            }
        }

        if (all_match == 1) {
            print_report(&report);
            printf("---------------------------------------\n\n");
        }
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

    // scriere in logged_district
    char logged_district_path[128];
    snprintf(logged_district_path, sizeof(logged_district_path), "%s/%s", path, "logged_district");

    int logged_district_file = open(logged_district_path, O_RDWR | O_APPEND);

    config_logged_district(logged_district_file, role, username, "filter");

    close(logged_district_file);
}

void remove_district(char *downtown, char *role, char *username) {
    if (strcmp(role, "manager") != 0) {
        printf("You don't have the permission!\n");
        return;
    }

    // path catre director
    char path[64];
    snprintf(path, sizeof(path), "%s/%s", "Districts", downtown);

    pid_t pid = fork();
    if (pid == 0) {
        execlp("bash", "bash", "script.sh", downtown, NULL);
        printf("execlp failed!\n");
        exit(1);
    }

    wait(NULL);

    // deleting symlink
    char symlink_path[128];
    snprintf(symlink_path, sizeof(symlink_path), "active_reports-%s", downtown);
    unlink(symlink_path);

    printf("Directory %s was deleted succesfully!\n", downtown);
}

// Observations:
// 1. Verify open/read/write/stat/mkdir/symlink
// 2. Filter function



