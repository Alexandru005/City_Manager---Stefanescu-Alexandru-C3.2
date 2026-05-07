#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

void handle_sigusr1(int sig) {
    printf("New report added!\n");
    fflush(stdout);
}

void handle_sigint(int sig) {
    printf("\nMonitor stopping...\n");
    unlink(".monitor_pid");
    exit(0);
}

int main() {

    // 2.1
    // create hidden file
    int file = open(".monitor_pid", O_RDWR | O_CREAT | O_TRUNC, 0644);

    // writing in hidden file
    pid_t pid = getpid();

    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d\n", pid);
    write(file, pid_str, strlen(pid_str));

    close(file);

    struct sigaction sa_usr1, sa_int;

    // handler SIGUSR1
    sa_usr1.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = 0;
    sigaction(SIGUSR1, &sa_usr1, NULL);

    // handler SIGINT
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    while(1) {
        pause();
    }

    return 0;
}
