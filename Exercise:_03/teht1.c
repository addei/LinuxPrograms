 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>  /*file control options*/
#include <unistd.h> /*POSIX standard symbolic constants and types*/

#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/stat.h>  /*käyttöoikeusbittimaskit*/
#include <errno.h>     /*makrot virheilmoituksia varten*/
#include <stdbool.h>    /*enabloi boolean datatyypin*/

/*virhetilanteen tulostus*/
int senderr(char *message, int errorcode, char *argv[]);
int sameFile(int fd1, int fd2);

int main(int argc, char *argv[]) {
    
    /*tarkistetaan argumenttien oikeellisuus*/
    switch (argc) {
        case 1:
            senderr("Annoit ohjelmalle liian monta argumenttiä. Yritä uudelleen!", EPERM, argv);
        case 3:
            break;
        default:
            senderr("Annoit ohjelmalle liian monta argumenttiä. Yritä uudelleen!", E2BIG, argv);
    };

    int fd1, fd2; /*tiedostokuvaajat*/

    if ((fd1 = open (argv[1], O_RDONLY, S_IRUSR | S_IRGRP)) < 0) {
            senderr("Tiedoston avaaminen ei onnistunut.", errno, argv);
    }

    if ((fd2 = open (argv[2], O_RDONLY, S_IRUSR | S_IRGRP)) < 0) {
            senderr("Tiedoston avaaminen ei onnistunut.", errno, argv);
    }

    int x = sameFile(fd1, fd2);

    close(fd1);
    close(fd2);

    /*tarkistetaan lopputulos*/
    switch (x) {
        case 0:
            printf("Tiedostot eivät viittaa toisiinsa!\n");
            break;
        case 1:
            printf("Tiedostot viittaavat samaan fyysiseen tiedostoon\n");
            break;
        default:
            senderr("Näin ei pitäisi käydä...", E2BIG, argv);
    };

    exit(EXIT_SUCCESS);

}

/*erillinen funktio virheviesteille*/
int senderr(char *message, int errorcode, char *argv[]) {
    fprintf(stderr, "%s\n", message);
    errno = errorcode;
    perror(argv[0]);
    exit(EXIT_FAILURE);
}


int sameFile(int fd1, int fd2) {
    
    struct stat fd1Stats;
    struct stat fd2Stats;

    if (fstat(fd1, &fd1Stats)) { 
        printf("\nfstat error: [%s]\n",strerror(errno));
    } 
    
    if (fstat(fd2, &fd2Stats)) { 
        printf("\nfstat error: [%s]\n",strerror(errno));
    }
    
    //st_dev tulee olla sama
    if (fd1Stats.st_dev == fd2Stats.st_dev) {
        //katso täsmääkö inode
        if (fd1Stats.st_ino == fd2Stats.st_ino) {
            //viittaa samaa tiedostoon.
            return true;
        }
    }
    return false;
}