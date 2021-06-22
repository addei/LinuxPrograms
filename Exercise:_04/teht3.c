#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/wait.h>
#include <errno.h>     /*makrot virheilmoituksia varten*/

int senderr(char *message, int errorcode, char *argv[]);


int main (int argc, char *argv[], char *env[]) {
    pid_t pid1 = -1, pid2 = -1;
    int wstatus1, wstatus2;

    /*luodaan kaksi lapsiprosessia samalle*/
    if ((pid1 = fork()) < 0) {
        senderr("Forkkaus epäonnistui", errno, argv);
    }

    if (pid1 == 0) {
        printf("Eka lapsi luotu!\n");
    }

    else {
        if ((pid2 = fork()) < 0) {
            senderr("Forkkaus epäonnistui", errno, argv);
        }

        if (pid2 == 0) {
            printf("Toka lapsi luotu!\n");
        }
    }

    /*lapsi1*/
    if (pid1 == 0) {
        printf("Eka lapsi valmis\n");
        exit(1);
    }

    /*lapsi2*/
    if (pid2 == 0) {
        sleep(3);
        printf("Toka lapsi valmis\n");
        exit(2);
    }



    /*emoprosessi*/
    if ((pid1 != 0) && (pid2 != 0)) {

        if (waitpid(pid2,&wstatus2,0) < 0) {
            senderr("waitpid epäonnistui", errno, argv);
        }

        if (WIFEXITED(wstatus2)) {
            printf("Toka lapsi exited statuksella=%d\n", WEXITSTATUS(wstatus2));
        }

        if (waitpid(pid1,&wstatus1,0) < 0) {
            senderr("waitpid epäonnistui", errno, argv);
        }
    
        if (WIFEXITED(wstatus1)) {
            printf("Eka lapsi exited statuksella=%d\n", WEXITSTATUS(wstatus1));
        }

    }

    exit(EXIT_SUCCESS);
}

//LAPSIPROSESSIN emoprosessin pid muuttu 1 = systemd (tulos saatu GNU/Linux OpenSUSE Tumbleweed jakelussa)

/*erillinen funktio virheviesteille*/
int senderr(char *message, int errorcode, char *argv[]) {
    fprintf(stderr, "%s\n", message);
    errno = errorcode;
    perror(argv[0]);
    exit(EXIT_FAILURE);
}