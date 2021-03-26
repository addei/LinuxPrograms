#include <unistd.h>  /* POSIX API */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#define MAXLINE 256

int main(void) {
    char buf[MAXLINE];
    pid_t pid;
    int status;

    printf("%% ");      /* tulosta kehoite: %%, jotta tulostuu % */
    while (fgets(buf, MAXLINE, stdin) != NULL) {
        
        buf[strlen(buf) - 1] = '\0'; /* korvaa rivinvaihtomerkki */

        if ((pid = fork()) < 0) {
            perror("fork error");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { /* lapsiprosessi jatkaa tästä */
            execlp(buf, buf, (char *)0);
            fprintf(stderr,"couldn't execute execlp: %s - %s", buf, strerror(errno));
            exit(EXIT_FAILURE);
        }

        /* mammaprosessi jatkaa tästä */
        if ((pid = waitpid(pid, &status, 0)) < 0) {
            perror("waitpid error");
            exit(EXIT_FAILURE);
        }
        printf("%% ");
    }
    exit(EXIT_SUCCESS);
}