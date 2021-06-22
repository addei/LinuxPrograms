#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>


int main (int argc, const char * argv[]) {
    int putki_v[2], putki_o[2];
    
    /*luodaan ensimmäinen (vasen) putki*/
    pipe(putki_v);

    // lapsiprosessi ps komennolle.
    int pid1 = fork();
    if (pid1 == 0) {
        close(putki_v[0]);

        if (dup2(putki_v[1], STDOUT_FILENO) < 0) {
            perror("dup2");
        }
        execlp("ps", "ps", "-A", NULL);
        exit(EXIT_SUCCESS);
    }

    /*luodaan toinen (oikea) putki*/
    pipe(putki_o);
    /*lapsiprosessi grep komennolle*/
    int pid2 = fork();
    if (pid2 == 0) {
        close(putki_v[1]);
        close(putki_o[0]);

        if (dup2(putki_v[0], STDIN_FILENO) < 0) {
            perror("dup2");
        }

        if (dup2(putki_o[1], STDOUT_FILENO) < 0) {
            perror("dup2");
        }
        
        /*tarkistetaan mikä on oletus shell jotta saadaa joku järkevä luku ulos*/
        const char* shell = getenv("SHELL");
        //fprintf(stderr, "%s\n", (shell!=NULL)? shell : "getenv returned NULL");

        /*bash shell*/
        if (strcmp(shell, "/usr/bin/bash") == 0) {
            execlp("grep", "grep", "bash", NULL);
        }

        /*zsh shell*/
        if (strcmp(shell, "/usr/bin/zsh") == 0) {
            execlp("grep", "grep", "zsh", NULL);
        }

        /*fish shell*/
        if (strcmp(shell, "/usr/bin/fish") == 0) {
            execlp("grep", "grep", "fish", NULL);
        }

        exit(EXIT_SUCCESS);
    }


    // lapsiprosessi wc komennolle.
    int pid3 = fork();
    if (pid3 == 0) {
        close(putki_o[1]);
        close(putki_v[1]);
        close(putki_v[0]);

        if (dup2(putki_o[0], STDIN_FILENO) < 0) {
            perror("dup2");
        }
        
        execlp("wc", "wc", "-l", NULL);
        exit(EXIT_SUCCESS);
    }
    

    //emo ei käytä putkia -> suljetaan
    close(putki_o[1]);
    close(putki_o[0]);
    close(putki_v[1]);
    close(putki_v[0]);

    /* jäädään odottamaan lapsiprosessin pid3 päättymistä */
    int status;
    waitpid(pid3, &status, 0);

    printf("EMO: Suoritus on päättynyt!\n");
    return 0;
}
