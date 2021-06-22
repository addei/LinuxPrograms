#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <errno.h>     /*makrot virheilmoituksia varten*/
#include <unistd.h>

#define buffersize 80
#define _XOPEN_SOURCE 500

int senderr(char *message, int errorcode, char *argv[]);
void signaalin_kasittelija(int sig, siginfo_t *info, void *ucontext);


int main (int argc, char *argv[], char *env[]) {

    //signaalin käsittely
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    /*laitetaan handlerille flagi*/
    act.sa_flags = SA_SIGINFO;
    /*määritetään handleri*/
    act.sa_sigaction = signaalin_kasittelija;
    
    
    /*määritetään signaalit toimimaan handlerin mukaisesti, ei talleneta vanhoja arvoja mihinkään*/
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);


    
    /*Looppi tulostaa joka sekuntti prosessinumeron*/
    while (1) {

        printf("Prosessinumero on: %d\n", getpid());
        sleep(1);

    }

    /*eipä tänne pitäisi päästä*/
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

/*signaalinkäsittelijä, ottaa parametrinä vastaanotetun signaalin tiedot.*/
void signaalin_kasittelija(int sig, siginfo_t *info, void *ucontext) {    
    if (sig == SIGINT) {
        char *signaali = "Tuli signaali SIGINT, mutta suoritus jatkuu\n";
        write(STDOUT_FILENO, signaali, strlen(signaali));
    }
    else if (sig == SIGTERM) {
        char *signaali = "Tuli signaali SIGTERM, mutta suoritus jatkuu\n";
        write(STDOUT_FILENO, signaali, strlen(signaali));
    }
}