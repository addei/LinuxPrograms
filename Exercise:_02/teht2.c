#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>  /*file control options*/
#include <unistd.h> /*POSIX standard symbolic constants and types*/

#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/stat.h>  /*käyttöoikeusbittimaskit*/
#include <errno.h>     /*makrot virheilmoituksia varten*/

/*struktuuri muuttujia varten*/
struct Tiedosto {
    char *name;
    struct stat stats; /*for file statistics*/
    int fd; /*file handling*/
    char *buf; /*malloc buf pointer*/
};

/*virhetilanteen tulostus*/
int senderr(char *message, int errorcode, char *argv);

int main(int argc, char *argv[]) {

    
    /*tarkistetaan argumenttien oikeellisuus*/
    switch (argc) {
        case 1:
            senderr("Ohjelma tarvitsee kaksi argumenttia tiedostojen muodossa", EINVAL, argv[0]);
        case 2:
            senderr("Ohjelma vaatii kaksi vertailtavaa tiedostoa!", EINVAL, argv[0]);
        case 3:
            break;
        default:
            senderr("Annoit ohjelmalle liian monta argumenttiä. Koita uudelleen!", E2BIG, argv[0]);
    };

    /*luodaan struktuurit tiedostoja varten*/
    struct Tiedosto tiedosto;
    struct Tiedosto tiedosto2;

    /*Otetaan argumenttien pointterit talteen struktuureihin*/
    tiedosto.name = argv[1];
    tiedosto2.name = argv[2];

    /*selvitetään tiedostojen koot*/
    stat(tiedosto.name, &tiedosto.stats);
    stat(tiedosto2.name, &tiedosto2.stats);

    /*Ilmoitetaan tiedostojen koot tavuissa*/
    printf("Tiedoston %s koko on %ld tavua\n", tiedosto.name, tiedosto.stats.st_size );
    printf("Tiedoston %s koko on %ld tavua\n", tiedosto2.name, tiedosto2.stats.st_size);

    /*jos tiedostot ovat erikokoiset (tavuiltaan) hylätään toiminto*/
    if (tiedosto.stats.st_size != tiedosto2.stats.st_size) {
        senderr("Tiedostot ovat kooltaan erikokoiset!", EPERM, argv[0]);
    }
    
    /*avataan tiedostot*/
    if ((tiedosto.fd = open (tiedosto.name, O_RDONLY)) < 0) {
        senderr("Tiedoston avaaminen ei onnistunut.", tiedosto.fd, argv[0]);
    }

    if ((tiedosto2.fd = open (tiedosto2.name, O_RDONLY)) < 0) {
        senderr("Tiedoston avaaminen ei onnistunut.", tiedosto.fd, argv[0]);
    }

    /*alustetaa dynaaminen muistialue tiedosto varten*/
    if ((tiedosto.buf = (char*) malloc(tiedosto.stats.st_size * sizeof(char *))) == NULL) {
        senderr("Muistin allokointi epäonnistui.", ENOMEM, argv[0]);
    }

    /*alustetaa dynaaminen muistialue tiedosto2 varten*/
    if ((tiedosto2.buf = (char*) malloc(tiedosto2.stats.st_size * sizeof(char *))) == NULL) {
        senderr("Muistin allokointi epäonnistui.", ENOMEM, argv[0]);
    }
    
    /*luetaan tiedosto dynaamiseen muistiin ja vapautetaan luku*/
    if (read(tiedosto.fd,tiedosto.buf,tiedosto.stats.st_size) != tiedosto.stats.st_size) {
        senderr("Luku tiedostosta epäonnistui.", ENOENT, argv[0]);
    }
    close(tiedosto.fd);

    /*luetaan tiedosto2 dynaamiseen muistiin vapautetaan luku*/
    if (read(tiedosto2.fd,tiedosto2.buf,tiedosto2.stats.st_size) != tiedosto2.stats.st_size) {
        senderr("Luku tiedostosta epäonnistui.", ENOENT, argv[0]);
    }
    close(tiedosto2.fd);

    /* luodaan lippu ja laskuri*/
    int flag = 0, count = 0;

    for (off_t x = 0; (x) < tiedosto.stats.st_size; x++) {
        if (tiedosto.buf[x] != tiedosto2.buf[x]) {
            printf("Eroavaisuus löytyi kohdasta %ld! %s: %c != %s: %c\n", x+1,  tiedosto.name, tiedosto.buf[x], tiedosto2.name, tiedosto2.buf[x]);
            flag = -1;
            count++;
        }
    }

    /*vapautetaan dynaamisesti varatut muistialueet*/
    free(tiedosto.buf);
    free(tiedosto2.buf);


    /*tulostetaan vertailun tulos*/
    if (flag == 0) {
        printf("Tiedostot ovat sisällöltään identtiset.\n");
    }

    else {
        printf("Tiedostot eivät ole identtisiä. Eroavaisuuksia havaittiin %d kappaletta\n", count);
    }

    exit(EXIT_SUCCESS);
}

/*erillinen funktio virheviesteille*/
int senderr(char *message, int errorcode, char *argv) {
    fprintf(stderr, "%s\n", message);
    errno = errorcode;
    perror(argv);
    exit(EXIT_FAILURE);
}
