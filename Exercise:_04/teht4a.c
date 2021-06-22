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
    int TempFile = -1;

    /*luodaan puskuri ja kirjoitetaan puskuriin nimi+xxxxxx satunnaisarvoijen merkiksi*/
    char nbuf[buffersize] = {'\0'};
    strncpy(nbuf, "valiaikaistiedosto-XXXXXX",26);
    /*luodaan väliaikaistiedosto ja kirjoitetaan puskurista vaaditut muuttujat*/
    if ((TempFile = mkstemp (nbuf)) < 0) {
            senderr("Väliaikaistiedoston luominen ei onnistunut.", errno, argv);
    }

    /*forkataan emoprosessi*/
    if ((pid = fork()) < 0) {
        senderr("Forkkaus epäonnistui!", errno, argv);
    }

    /*lapsi jatkaa tästä*/
    if (pid == 0) {
        
        /*luodaan oma puskuri lapsiprosessille*/
        char fbuf[buffersize] = {'\0'};
        /*kirjoitetaan string puskuriin*/
        strncpy(fbuf, "Lapsiprosessi kirjoitti tekstiä valiaikaistiedostoon!\n:)\n", 59);

        /*kirjoitetaan väliaikaistiedostoon puskurista teksti*/
        if (-1 == write(TempFile,fbuf,sizeof(fbuf))) {
            exit(1);
        }
        //unlink(nbuf); /*lapsi hävittää tiedoston*/
        exit(0);
    }

    /*emoprosessi jatkaa tästä*/
    else {
        /*oottaa että lapsiprosessi on valmis*/
        if (waitpid(pid,&wstatus,0) < 0) {
            senderr("waitpid epäonnistui", errno, argv);
        }
        /*jos lapsiprosessin suoritus onnistui -> laita hakupää alkuun ja tulosta tiedoston puskuriin ja siitä näytölle*/
        if (WEXITSTATUS(wstatus) == 0) {
            lseek(TempFile, 0, SEEK_SET);
            char buf[buffersize] = {'\0'};
            if (read(TempFile, buf, buffersize) == 0) {
                senderr("Luku tiedostosta epäonnistui!", ENOENT, argv);
            }
            printf("%s", buf);
        }

        else {
            senderr("Väliaikaistiedostoon kirjoittaminen epäonnistui!", errno, argv);
        }
    }
    /*vapauta väliaikaistiedosto*/
    unlink(nbuf);
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