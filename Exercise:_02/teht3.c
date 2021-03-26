 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>  /*file control options*/
#include <unistd.h> /*POSIX standard symbolic constants and types*/

#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/stat.h>  /*käyttöoikeusbittimaskit*/
#include <errno.h>     /*makrot virheilmoituksia varten*/
#include <ctype.h>     /*isdigit*/

#define buffersize 4096 /*putken koko oletuksena*/

/*virhetilanteen tulostus*/
int senderr(char *message, int errorcode, char *argv);

int main(int argc, char *argv[]) {
    
    /*tarkistetaan argumenttien oikeellisuus*/
    switch (argc) {
        case 2:
            break;
        default:
            senderr("Annoit ohjelmalle liian monta argumenttiä. Yritä uudelleen!", E2BIG, argv[0]);
    };
    
    /*luodaan muuttujat*/
    int fd;
    char buffer[buffersize] = {'\0'};

    if ((fd = open (argv[1], O_APPEND | O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR | S_IRGRP)) < 0) {
        senderr("Tiedoston avaaminen ei onnistunut.", EPERM, argv[0]);
    }

    printf("Anna kokonaislukuja!\n");

    /*kirjoittaa puskurista saadun tekstin tiedostoon.*/
    while (read(STDIN_FILENO, &buffer, buffersize)) {

        /*alustaa lipun*/
        int flag = 0;

        /*tarkistaa onko kaikki puskurista saadut arvot kokonaislukuja tai rivinvaihto*/
        for (int x = 0; x != strlen(buffer); x++) {
            if ( (isdigit(buffer[x]) | (buffer[x] == '\n') | (buffer[x] == '-') ) == 0) {
                printf("Anna syötteenä vain kokonaislukuja!\n");
                flag = -1;
                break;
            }
        
            /*tarkistaa löytyykö rivinvaihto -> jos löytyy nii break*/
            if ( (buffer[x] == '\n') ) {
                buffer[x] = '\0';
                break;
            }
        }

        /*jos lippu pysyy arvossaan -> kirjoita tiedostoon.*/
        if (flag == 0) {
            write(fd, buffer, strlen(buffer));
            write(fd, "\n", 1);
        }
    }
    close(fd);
    exit(EXIT_SUCCESS);
}

/*erillinen funktio virheviesteille*/
int senderr(char *message, int errorcode, char *argv) {
    fprintf(stderr, "%s\n", message);
    errno = errorcode;
    perror(argv);
    exit(EXIT_FAILURE);
}
