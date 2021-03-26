#include <unistd.h>  /* POSIX API */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#define MAXLINE 256

int main(void) {
    char buf[(MAXLINE * 2)];
    pid_t pid;
    int status;

    fprintf(stdout, ">> ");      /* tulosta kehoite: >>, jotta tulostuu >> */
    while (fgets(buf, MAXLINE, stdin) != NULL) {
        
        buf[strlen(buf) - 1] = '\0'; /*korvaa rivinvaihtomerkki*/

        /*tähä tulee komennon käsittelyyn käytetyt muuttujat*/
        char command[MAXLINE];
        char arguments[MAXLINE] = {'\0'};
        const char delimiter[2] = " ";
        
        
        char *token = strtok(buf, delimiter); /*pilkotaan string tokeneiksi*/

        strcpy(command, token); /*kopioi ensimmäisen tokenin komentolistaan*/

        while( token != NULL ) { /*appendaa argumentit listaan*/
            token = strtok(NULL, delimiter);
            if (token != NULL) {
                strcat(arguments, token);
                strcat(arguments, delimiter);
            }
            
        }

        arguments[strlen(arguments) - 1] = '\0'; /* korvaa välilyöntimerkin lopusta */

        if ((pid = fork()) < 0) {
            perror("fork error");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { /* lapsiprosessi jatkaa tästä */
            
            /*vertaa onko argumentteja annettu vai ei*/
            if (strlen(arguments) == 0) {
                execlp(command, command, NULL);
            }
            
            else {
                execlp(command, command, arguments, NULL);
            }
            
            fprintf(stderr,"couldn't execute execlp: %s - %s", buf, strerror(errno));
            exit(EXIT_FAILURE);
        }

        /* mammaprosessi jatkaa tästä */
        if ((pid = waitpid(pid, &status, 0)) < 0) {
            perror("waitpid error");
            exit(EXIT_FAILURE);
        }
        fprintf(stdout, ">> ");
    }
    exit(EXIT_SUCCESS);
}