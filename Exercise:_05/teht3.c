#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <errno.h>     /*makrot virheilmoituksia varten*/
#include <unistd.h>

#define time 1
#define _XOPEN_SOURCE 500 

int senderr(char *message, int errorcode, char *argv[]);
void signaalin_kasittelija(int sig, siginfo_t *info, void *ucontext);

/*globaalit muuttujat*/
int i = 3;


int main (int argc, char *argv[], char *env[]) {
 
    pid_t pid1, pid2, pid3;

    //signaalin käsittely
    struct sigaction act;
    /*määritetään handleri*/
    act.sa_sigaction = signaalin_kasittelija;
    /*laitetaan handlerille flagi*/
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    /*määritetään signaalit toimimaan handlerin mukaisesti, ei talleneta vanhoja arvoja mihinkään*/
    sigaction(SIGCHLD, &act, NULL);

    printf("Hei olen ohjelma jonka oma prosessinumeroni on %d\nSeuraavaksi luon kolme lapsiprosessia ja jään odottamaan kolmea \"SIGCHLD\" signaaleja!\n\n", getpid());
    sleep(1);
    /*forkataan emoprosessi*/
    if ((pid1 = fork()) < 0) {
        senderr("Forkkaus epäonnistui!", errno, argv);
    }
    /*lapsi1 jatkaa tästä*/
    if (pid1 == 0) {
        sleep(time);
        exit(EXIT_SUCCESS);
    }

    else {
        if ((pid2 = fork()) < 0) {
            senderr("Forkkaus epäonnistui!", errno, argv);
        }
        /*lapsi2 jatkaa tästä*/
        if (pid2 == 0) {
            sleep(time + 1);
            exit(EXIT_SUCCESS);
        }

        else {
            if ((pid3 = fork()) < 0) {
                senderr("Forkkaus epäonnistui!", errno, argv);
            }
            /*lapsi3 jatkaa tästä*/
            if (pid3 == 0) {
                sleep(time + 2);
                exit(EXIT_SUCCESS);
            }
                /*emoprosessi jatkaa tästä*/
            else {
        
                int counter = 0;
                do {
                    printf("%d\n", counter);
                    counter++;
                    sleep(1);
                } while (i);
                exit(EXIT_SUCCESS);
            }
        }
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
    char *signaali = "Tuli signaali SIGCHLD (EMO)\n";
    write(STDOUT_FILENO, signaali, strlen(signaali));
    i--;
}
