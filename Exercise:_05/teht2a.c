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
    /*määritetään handleri*/
    act.sa_sigaction = signaalin_kasittelija;
    /*laitetaan handlerille flagi*/
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    /*määritetään signaalit toimimaan handlerin mukaisesti, ei talleneta vanhoja arvoja mihinkään*/
    sigaction(SIGUSR1, &act, NULL);


    printf("Hei olen ohjelma jonka oma prosessinumeroni on %d\nKolmen sekunnin päästä lähetän emoprosessille (%d) SIGUSR1 signaalin!\n", getpid(), getppid());
    int i = 3;
    do {
        printf("%d\n", i);
        sleep(1);
        i--;
    } while(i);
    
    kill(getppid(), SIGUSR1);
    pause();
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
    char *signaali = "Tuli signaali SIGUSR1 (LAPSI)\n";
    write(STDOUT_FILENO, signaali, strlen(signaali));
    raise(SIGTERM);
}
