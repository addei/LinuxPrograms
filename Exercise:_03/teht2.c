#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>  /*file control options*/
#include <unistd.h> /*POSIX standard symbolic constants and types*/

#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/stat.h>  /*käyttöoikeusbittimaskit*/
#include <errno.h>     /*makrot virheilmoituksia varten*/
#include <ctype.h>
#include <float.h>
#include <stdbool.h>    /*enabloi boolean datatyypin*/

/*virhetilanteen tulostus*/
int senderr(char *message, int errorcode, char *argv[]);
int etsi(int fd, char *mjono, char* prgname[]);

int main(int argc, char *argv[]) {
    
    /*tarkistetaan argumenttien oikeellisuus*/
    switch (argc) {
        case 1:
            senderr("Ohjelma merkkijonon ja tiedostonimiä!", EAGAIN, argv);
        case 2:
            senderr("Ohjelma tarvitsee tiedostonimiä!", EAGAIN, argv);
        case 3:
            break;
        default:
            senderr("Ohjelmalle annettiin liian monta argumenttia", E2BIG, argv);
    };

    /*luodaan tiedostokuvaaja*/
    int fd;
    /*avataan tiedostot*/
    if ((fd = open (argv[2], O_RDONLY)) < 0) {
        senderr("Tiedoston avaaminen ei onnistunut.", errno, argv);
    }

    /*etsitään haluttu merkkijono tiedostosta*/
    int x = etsi(fd, argv[1], argv);
    /*suljetaan avattu tiedosto*/
    close(fd);

    /*tarkistetaan lopputulos*/
    switch (x) {
        case 0:
            printf("Merkkijono \"%s\" EI löytynyt tiedostosta \"%s\"!\n", argv[1], argv[2]);
            break;
        case 1:
            printf("Merkkijono \"%s\" löytyi tiedostosta \"%s\"!\n", argv[1], argv[2]);
            break;
        default:
            senderr("Näin ei pitäisi käydä...", E2BIG, argv);
    };

    /*palautetaan "EXIT_SUCCESS" suorituksen onnistumisen merkiksi*/
    exit(EXIT_SUCCESS);
}

/*erillinen funktio virheviesteille*/
int senderr(char *message, int errorcode, char *argv[]) {
    fprintf(stderr, "%s\n", message);
    errno = errorcode;
    perror(argv[0]);
    exit(EXIT_FAILURE);
}

/*funktio etsii annetusta tiedostokuvaajasta annetun merkkijonon ja palauttaa 0/1 arvon*/
int etsi(int fd, char *mjono, char* prgname[]) {

    struct stat Filestats; /*luodaan rakenne tiedostotilastoja varte*/
    fstat(fd, &Filestats); /*kopioidaan arvot edelläluotuun rakenteeseen järjestelmän tietorakenteista*/
    char* buf; /*osoitin dynaamista muistia varten*/

    /*alustetaa dynaaminen muistialue tiedostoa varten*/
    if ((buf = (char*) malloc ( ( Filestats.st_size * sizeof(char *) ) + 1 ) ) == NULL) {
        senderr("Muistin allokointi epäonnistui.", errno, prgname);
    }

    /*luetaan tiedosto dynaamiseen muistiin ja vapautetaan luku*/
    if (read(fd,buf,Filestats.st_size) != Filestats.st_size) {
        senderr("Luku tiedostosta epäonnistui.", errno, prgname);
    }
    close(fd);

    /*laitetaan dynaamisen muistin taulukon viimeisen elementin arvoksi "\0"*/
    buf[-1] = '\0';

    /*luodaan pointteri ja etsitään taulukosta merkkijono*/
    char* ptr;
    ptr = strstr(buf, mjono);

    /*vapautetaan dynaaminen muisti*/
    free(buf);

    /*palautetaan funktiosta tulos*/
    if (ptr == NULL) {
        return false;
    }
    else {
        return true;
    }
}