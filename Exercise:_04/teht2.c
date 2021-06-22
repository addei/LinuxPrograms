#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/wait.h>
#include <errno.h>     /*makrot virheilmoituksia varten*/

int senderr(char *message, int errorcode, char *argv[]);


int main (int argc, char *argv[], char *env[]) {
    pid_t pid;


    if ((pid = fork()) < 0) {
        senderr("Forkkaus epäonnistui", errno, argv);
    } 

    if (pid == 0) {
        printf("LAPSIPROSESSI -> Prosessi numero: pid=%d, Emoprosessin numero: pid=%d\n", getpid(), getppid());
        sleep(10);
        printf("LAPSIPROSESSI -> Prosessi numero: pid=%d, Emoprosessin numero: pid=%d\n", getpid(), getppid());
        exit(EXIT_SUCCESS);
    }
    else {
        printf("EMOPROSESSI -> Prosessi numero: pid=%d, Emoprosessin numero: pid=%d\n", getpid(), getppid());
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