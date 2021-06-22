#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/wait.h>
#include <errno.h>     /*makrot virheilmoituksia varten*/
#include <fcntl.h>

#define buffersize 80
#define _XOPEN_SOURCE 500 

int senderr(char *message, int errorcode, char *argv[]);
void signaalin_kasittelija(int sig, siginfo_t *info, void *ucontext);

int main (int argc, char *argv[], char *env[]) {
    pid_t pid;
    int putki[2];

    /*luodaan putki*/
    if (pipe(putki) < 0) {
        senderr("Putken luominen epäonnistui!", errno, argv);
    }

    /*luodaan lapsiprosessi*/
    if ((pid = fork()) < 0) {
        senderr("Forkkaus epäonnistui!", errno, argv);
    }

    /*lapsiprosessi*/
    if (pid == 0) {
        /*suljetaan lapsiprosessin luku*/
        close(putki[0]);
        /*alustetaan puskuri*/
        char buf[buffersize] = {'\0'};
        /*luodaa timer*/
        int laskuri = 20;
        do {
            /*kirjoitetaan puskuriin*/
            strncpy(buf, "Lapsiprosessi kirjoitti tekstiä putkeen! :)\n", 46);
            /*odottaa sekunnin*/
            sleep(1);
            /*kirjoittaa putkeen puskurin datan*/
            write(putki[1], buf, strlen(buf));
            laskuri--;
        } while (laskuri);
        /*putket kiinni*/
        close(putki[1]);
        /*suljetaan lapsiprosessi*/
        exit(EXIT_SUCCESS);
    }

    /*emoprosessi*/
    else {

        //signaalin käsittely
        struct sigaction act;
        /*määritetään handleri*/
        act.sa_sigaction = signaalin_kasittelija;
        /*laitetaan handlerille flagi*/
        act.sa_flags = SA_SIGINFO;
        sigemptyset(&act.sa_mask);
        /*määritetään signaalit toimimaan handlerin mukaisesti, ei talleneta vanhoja arvoja mihinkään*/
        sigaction(SIGCHLD, &act, NULL);

        /*suljetaan emoprosessilta kirjoitus putkeen*/
        close(putki[1]);

        //muutetaan stdin ja putken bittiflagit
        /*kopioidaan jo olemassa olevat flagit*/
        int plippu = fcntl(putki[0], F_GETFL);
        int slippu = fcntl(STDIN_FILENO, F_GETFL);
        /*lisätään O_NONBLOCK bitit*/
        fcntl(putki[0], F_SETFL, plippu | O_NONBLOCK);
        fcntl(STDIN_FILENO, F_SETFL, slippu | O_NONBLOCK);

        /*aloitetaan looppaaminen*/
        while (1) {
            /*alustetaan puskuri*/
            char buf[buffersize] = {'\0'};
            
            /*luetaan eka putkesta puskurin verran dataa*/
            if (read(putki[0], &buf, buffersize) < 0) {
                /*tarkastaa jos putki ei ilmoitta EAGAIN -> jos totta ilmoittaa virheen ja lopettaa prosessin ajamisen*/
                if (errno != EAGAIN) {
                    senderr("Virhe putken lukemisessa", errno, argv);
                }
            }
            /*jos on voitu lukea dataa, kirjoittaa sen stdout*/
            else {
                    write(STDOUT_FILENO, &buf, strlen(buf));
            }
            
            /*luetaan stdin puskurin verran dataa*/
            if (read (STDIN_FILENO, &buf, buffersize) < 0) {
                /*tarkastaa jos putki ei ilmoitta EAGAIN -> jos totta ilmoittaa virheen ja lopettaa prosessin ajamisen*/
                if (errno != EAGAIN) {
                    senderr("Virhe stdin lukemisessa", errno, argv);
                }
            }
            /*jos on voitu lukea dataa, kirjoittaa sen stdout*/
            else {
                    write(STDOUT_FILENO, &buf, strlen(buf));
            }
        }
    }

    exit(EXIT_SUCCESS);
}

/*erillinen funktio virheviesteille*/
int senderr(char *message, int errorcode, char *argv[]) {
    fprintf(stderr, "%s\n", message);
    errno = errorcode;
    perror(argv[0]);
    exit(EXIT_FAILURE);
}

/*signaalinkäsittelijä, ottaa parametrinä vastaanotetun signaalin tiedot.*/
void signaalin_kasittelija(int sig, siginfo_t *info, void *ucontext) {    
    char *signaali = "Tuli signaali SIGCHLD -> lopetetaan ohjelman ajo!\n";
    write(STDOUT_FILENO, signaali, strlen(signaali));
    exit(EXIT_SUCCESS);
}
