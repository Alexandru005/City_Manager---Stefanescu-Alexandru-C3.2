#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"

int main(int argc, char **argv) {

    if (argc < 3) {
        printf("Argumente insuficiente!\n");
        exit(1);
    }

    char role[25];
    char username[50];

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0) {
            set_role(argv[i + 1], role);
        }else if (strcmp(argv[i], "--user") == 0) {
            set_username(argv[i + 1], username);
        }else if (strcmp(argv[i], "--add") == 0) {
            add(argv[i + 1], role, username);
        }else if (strcmp(argv[i], "--list") == 0) {
            list(argv[i + 1], role, username);
        }else if (strcmp(argv[i], "--view") == 0) {
            view(argv[i + 1], argv[i + 2], role, username);
        }else if (strcmp(argv[i], "--remove_report") == 0) {
            remove_report(argv[i + 1], argv[i + 2], role, username);
        }else if (strcmp(argv[i], "--update_threshold") == 0) {
            update_threshold(argv[i + 1], argv[i + 2], role, username);
        }else if (strcmp(argv[i], "--remove_district") == 0) {
            remove_district(argv[i + 1], role, username);
        }
    }




    return 0;
}
