#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    char command[256];

    while (1) {
        printf("> ");
        if (fgets(command, sizeof(command), stdin) == NULL) break;
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "start_monitor") == 0) {
            int pfd[2];
            pipe(pfd);

            pid_t hub_mon = fork();
            if (hub_mon == 0) {
                pid_t mon_pid = fork();
                if (mon_pid == 0) {
                    close(pfd[0]);
                    dup2(pfd[1], STDOUT_FILENO); // Redirectăm stdout în pipe
                    close(pfd[1]);
                    execlp("./monitor_reports", "monitor_reports", NULL);
                    exit(1);
                }

                close(pfd[1]);
                char buf[256];
                int n;
                // Citim din pipe cât timp monitorul produce output
                while ((n = read(pfd[0], buf, sizeof(buf)-1)) > 0) {
                    buf[n] = '\0';
                    printf("[Monitor]: %s", buf);
                    fflush(stdout);

                    // Verificăm dacă mesajul indică închiderea sau o eroare
                    if (strstr(buf, "stopping") != NULL || strstr(buf, "ERROR") != NULL) {
                        printf("[Hub Info]: Procesul monitor a fost încheiat.\n");
                    }
                }
                close(pfd[0]);
                exit(0);
            }
            close(pfd[0]);
            close(pfd[1]);

        } else if (strncmp(command, "calculate_scores", 16) == 0) {
            char *district = strtok(command, " ");
            district = strtok(NULL, " "); // Luăm primul district

            while (district != NULL) {
                int pfd[2];
                pipe(pfd);

                pid_t pid = fork();
                if (pid == 0) {
                    // Proces copil (Scorer)
                    close(pfd[0]);
                    dup2(pfd[1], STDOUT_FILENO); // Redirectăm stdout în pipe
                    close(pfd[1]);

                    // Lansăm executabilul extern
                    execlp("./scorer", "scorer", district, NULL);
                    perror("execlp failed"); // Afișat doar dacă execlp eșuează
                    exit(1);
                }

                // Proces părinte (Hub)
                close(pfd[1]);
                char buf[512];
                int n;

                printf("=== Workload Report for District: %s ===\n", district);
                // Citim rezultatele din pipe de la scorer
                while ((n = read(pfd[0], buf, sizeof(buf) - 1)) > 0) {
                    buf[n] = '\0';
                    printf("%s", buf);
                }
                close(pfd[0]);
                wait(NULL); // Așteptăm să termine scorer-ul curent

                printf("============================================\n\n");

                district = strtok(NULL, " ");
            }

        } else if (strcmp(command, "exit") == 0) {
            break;
        }
    }

    return 0;
}