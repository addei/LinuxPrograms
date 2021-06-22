#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/wait.h>
#include <errno.h>     /*makrot virheilmoituksia varten*/

#define buffersize 80
#define _XOPEN_SOURCE 500 

int senderr(char *message, int errorcode, char *argv[]);


int main (int argc, char *argv[], char *env[]) {
    pid_t pid;
    int wstatus;    
    int putki[2];

    /*luodaan putki*/
    if (pipe(putki) < 0) {
        senderr("Putken luominen epäonnistui!", errno, argv);
    }

    /*luodaan lapsiprosessi*/
    if ((pid = fork()) < 0) {
        senderr("Forkkaus epäonnistui!", errno, argv);
    }

    /*lapsi*/
    if (pid == 0) {
        /*suljetaan lapsiprosessin luku*/
        close(putki[0]);
        /*alustetaan puskuri*/
        char buf[buffersize] = {'\0'};
        /*kirjoitetaan puskuriin*/
        strncpy(buf, "Lapsiprosessi kirjoitti tekstiä putkeen!\n:)\n", 46);
        /*kirjoittaa putkeen puskurin datan*/
        write(putki[1], buf, strlen(buf));
        /*putket kiinni*/
        close(putki[1]);
        /*suljetaan lapsiprosessi*/
        exit(0);
    }

    else {
        /*suljetaan emoprosessilta kirjoitus putkeen*/
        close(putki[1]);
        /*alustetaan puskuri*/
        char buf[buffersize] = {'\0'};

        /*odotetaan että lapsiprosessi on lopetattunut suorituksensa*/
        if (waitpid(pid,&wstatus,0) < 0) {
            senderr("waitpid epäonnistui", errno, argv);
        }
        if (WEXITSTATUS(wstatus) == 0) {
            /*lukee putkesta*/
            while (read(putki[0], &buf, 1) > 0) {
                write(STDOUT_FILENO, &buf, 1);
            }
        }

        else {
            senderr("Putkeen kirjoittaminen epäonnistui!", errno, argv);
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