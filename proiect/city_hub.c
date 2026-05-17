#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main() {



    char command[256];

    while (1) {
        printf("> ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "start_monitor") == 0) {
            int pfd[2];
            pipe(pfd);

            pid_t hub_mon = fork();  // primul fork - creeaza hub_mon
            if (hub_mon == 0) {
                // acesta e hub_mon
                pid_t mon_pid = fork();  // al doilea fork - creeaza monitor_reports
                if (mon_pid == 0) {
                    close(pfd[0]);
                    dup2(pfd[1], STDOUT_FILENO);
                    close(pfd[1]);
                    execlp("./monitor_reports", "monitor_reports", NULL);
                    exit(1);
                }

                close(pfd[1]);
                char buf[256];
                int n;
                while ((n = read(pfd[0], buf, sizeof(buf)-1)) > 0) {
                    buf[n] = '\0';
                    printf("[Monitor]: %s", buf);
                    fflush(stdout);
                }
                exit(0);
            }
            // parintele (city_hub) continua bucla
            close(pfd[0]);
            close(pfd[1]);
        }
        else if (strcmp(command, "exit") == 0) {
            break;
        }
    }

    return 0;
}