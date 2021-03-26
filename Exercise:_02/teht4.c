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

#define charbuffersize 2
#define buffersize 4096 /*putken oletuskoko*/

/*virhetilanteen tulostus*/
int senderr(char *message, int errorcode, char *argv[]);
int addints(int size, char* filename[]);
int calcsum(int size, char* filename[]);

int main(int argc, char *argv[]) {
    
    /*tarkistetaan argumenttien oikeellisuus*/
    switch (argc) {
        case 2:
            break;
        default:
            senderr("Annoit ohjelmalle liian monta argumenttiä. Yritä uudelleen!", E2BIG, argv);
    };

    struct stat fileStats; /*for file statistics*/
    
    /*tarkistetaan tiedoston olemassaolo, koko, ja oikeudet.*/
    if ( stat(argv[1], &fileStats) < 0 ) {
        /*jos tiedostoa ei ole olemassa, luodaan se*/
        int fd;
        if ((fd = open (argv[1], O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR | S_IRGRP)) < 0) {
            senderr("Tiedoston luominen ei onnistunut.", EPERM, argv);
        }
        /*suljetaan luotu tiedosto*/
        close(fd);

        /*lisätään kokonaislukuja juuriluotuun tiedostoon*/
        addints(argc, argv);
    }

    
    else {
        
        printf("Tiedosto on olemassa.\n\n");

        int toggle = 0;
        while (toggle == 0) {
            printf("Haluatko korvata olemassa olevan tiedoston %s (y/n)?\n", argv[1]);
            int fd;
            char buffer[buffersize] = {'\0'};
            read(STDIN_FILENO, &buffer, buffersize);

            switch (buffer[0]) {
                case 'y':
                    if ((fd = open (argv[1], O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR | S_IRGRP)) < 0) {
                        senderr("Tiedoston alustaminen ei onnistunut.", EPERM, argv);
                    }
                    /*suljetaan luotu tiedosto*/
                    close(fd);
                    toggle--;
                    printf("\n\n");
                    break;
                case 'n':
                    toggle--;
                    printf("\n\n");
                    break;
                case 'q':
                    exit(EXIT_SUCCESS);
                case 'Q':
                    exit(EXIT_SUCCESS);
                case '?':
                    printf("Anna komento muodossa \"y/n\". Ohjelmasta voi poistua painamalla \"q\"\n");
                    break;
            };
        }
    }

    /*switch case. 1 joko calcsum, addints tai quit*/
    while (1) {
            printf("VALITSE TOIMINTO\n");
            printf("1: Laske tiedostossa olevien kokonaislukujen summa ja tallenna se viimeiselle riville?\n");
            printf("2: Lisää tiedostoon kokonaislukuja?\n");
            printf("q: Poistu ohjelmasta?\n");
            
            char buffer[buffersize] = {'\0'};
            read(STDIN_FILENO, &buffer, buffersize);
            
            switch (buffer[0]) {
                case '1':
                    calcsum(argc, argv);
                    break;
                case '2':
                    addints(argc, argv);
                    break;
                case 'q':
                    exit(EXIT_SUCCESS);
                case 'Q':
                    exit(EXIT_SUCCESS);
                case '?':
                    printf("Anna komento muodossa \"1\",\"2\" tai \"q\".\n");
            };
    }
}

/*erillinen funktio virheviesteille*/
int senderr(char *message, int errorcode, char *argv[]) {
    fprintf(stderr, "%s\n", message);
    errno = errorcode;
    perror(argv[0]);
    exit(EXIT_FAILURE);
}

/*lisää kokonaislukuja annettuun tiedostoon rivi riviltä*/
int addints(int size, char* filename[]) {

    int fd;

    if ((fd = open (filename[1], O_APPEND | O_WRONLY, S_IWUSR | S_IRUSR | S_IRGRP)) < 0) {
        senderr("Tiedoston avaaminen ei onnistunut.", ENOENT, filename);
    }
    char buffer[buffersize] = {'\0'};
    printf("Anna kokonaislukuja! Paina \"q\" poistuaksesi.\n");

    /*kirjoittaa puskurista saadun tekstin tiedostoon.*/
    while (read(STDIN_FILENO, &buffer, buffersize)) {

        if ( (buffer[0] == 'q') | (buffer[0] == 'Q') ) {
            printf("\n\n");
            break;
        }

        /*alustaa lipun*/
        int flag = 0;

        /*tarkistaa onko kaikki puskurista saadut arvot kokonaislukuja, rivinvaihto tai miinusmerkki*/
        for (int x = 0; x != strlen(buffer); x++) {
            if ( (isdigit(buffer[x]) | (buffer[x] == '\n') | ( buffer[x] == '-') ) == 0) {
                printf("Anna syötteenä vain kokonaislukuja!. Paina \"q\" poistuaksesi.\n");
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
    return 0;
}

/*funktio joka lukee annetusta tiedostosta riveiltä löytyvät kokonaisluvut, laskee yhteen ja tallentaa annetun tiedoston loppuun*/
int calcsum(int size, char* filename[]) {

    /*muuttujat tiedostonhallintaa varten*/
    int fd;
    char *buf;
    struct stat Filestats; /*for file statistics*/
    
    stat(filename[1], &Filestats);

    if (Filestats.st_size == 0) {
        printf("Lisää tiedostoon %s kokonaislukuja ensin!\n\n", filename[1]);
        return 0;
    }

    /*avataan tiedostot*/
    if ((fd = open (filename[1], O_RDONLY)) < 0) {
        senderr("Tiedoston avaaminen ei onnistunut.", ENOENT, filename);
    }

    /*alustetaa dynaaminen muistialue tiedostoa varten*/
    if ((buf = (char*) malloc ( Filestats.st_size * sizeof(char *) ) ) == NULL) {
        senderr("Muistin allokointi epäonnistui.", ENOMEM, filename);
    }

    /*luetaan tiedosto dynaamiseen muistiin ja vapautetaan luku*/
    if (read(fd,buf,Filestats.st_size) != Filestats.st_size) {
        senderr("Luku tiedostosta epäonnistui.", ENOENT, filename);
    }
    close(fd);

    /*muuttujat laskentaa varten*/
    float value = 0;
    long index = 0, rcount = 0;
    char cbuf[buffersize] = {'\0'};

    /*käydään char array läpi*/
    for (off_t x = 0; (x) < Filestats.st_size; x++) {
        
        cbuf[index] = buf[x];
        
        if ( isdigit(cbuf[index]) | (cbuf[index] == '-') ) {
            index++;
        }

        if ( buf[x] == '\n' )  {
            cbuf[index] = '\0';
            value = value + atof(cbuf);
            rcount++;
            index = 0;
        }        
    }

    /*vapautetaan dynaaminen muistinvaraus*/
    free(buf);

    /*testi onko luku laskettavissa ohjelmiston avulla*/
    if (FLT_MAX < value) {
        /*tulostetaan rivimäärä ja laskutoimitus*/
        printf("%ld rivin yhteenlaskettu summa on: %.0f\n", rcount, value);
        printf("Arvo on liian suuri käsiteltäväksi. Arvoa ei tallenneta levylle.\n\n");
    }

    else if (value == 0) {
        printf("%s ei sisältänyt yhteen käsiteltävää kokonaislukua!\n\n", filename[1]);
    }

    else {
        /*tulostetaan rivimäärä ja laskutoimitus*/
        printf("%ld rivin yhteenlaskettu summa on: %.0f\n", rcount, value);
        printf("Lisätään arvo tiedoston loppuun...\n");
    
        if ((fd = open (filename[1], O_APPEND | O_WRONLY, S_IWUSR | S_IRUSR | S_IRGRP)) < 0) {
            senderr("Tiedoston avaaminen ei onnistunut.", ENOENT, filename);
        }

        /*tallentetaa laskettu arvo tiedostoon*/
        sprintf(cbuf, "%.0f", value);
        printf("\n\n");
        write(fd, cbuf, strlen(cbuf));
        write(fd, "\n", 1);

    }
    /*suljetaan tiedosto*/
    close(fd);
    return 0;
}