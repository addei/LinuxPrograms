#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/wait.h>
#include <errno.h>     /*makrot virheilmoituksia varten*/
#include <sys/select.h>

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

        /*bittimaskit select system call varten*/
        fd_set r_flags;
        int sel;
        FD_ZERO(&r_flags);
        /*asetetaan seurattavat osiot*/
        FD_SET(STDIN_FILENO, &r_flags);
        FD_SET(putki[0], &r_flags);

        /*aloitetaan looppaaminen*/
        while ((sel = select(putki[0] + 1, &r_flags, NULL, NULL, NULL)) > 0) {
            
            if (FD_ISSET(STDIN_FILENO, &r_flags)) {
                char sbuf[buffersize] = {'\0'};
                read(STDIN_FILENO, &sbuf, buffersize);
                write(STDOUT_FILENO, &sbuf, strlen(sbuf));
            }
            
            /*luetaan eka putkesta puskurin verran dataa*/
            if (FD_ISSET(putki[0], &r_flags)) {
                char pbuf[buffersize] = {'\0'};
                read(putki[0], &pbuf, buffersize);
                write(STDOUT_FILENO, &pbuf, strlen(pbuf));
            }

            /*jos on voitu lukea dataa, kirjoittaa sen stdout*/
            if (sel == -1) {
                senderr("Virhe synkronoinnin kanssa!", errno, argv);
            }
            /*alustetaan maskit ja asetetaan arvot uudelleen*/
            FD_ZERO(&r_flags);
            FD_SET(STDIN_FILENO, &r_flags);
            FD_SET(putki[0], &r_flags);
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
