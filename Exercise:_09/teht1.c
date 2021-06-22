//includes
#include <stdio.h> /*stdin - stdout*/
#include <stdlib.h> /*standard library definitions*/
#include <unistd.h> /*posix api*/
#include <math.h> /*C math library*/
#include <pthread.h> /*posix library for threading*/
#include <errno.h> /*error api*/

//defines
#define SIZE 90

//global variables
float neliojuuret[SIZE];

//function declarations
int senderr(char *message, int errorcode, char *argv[]);


//thread declarations
void *thr_nsqrtf(void *arg);

int main (int argc, char *argv[], char *env[]) {
    /*tarkistetaan argumenttien oikeellisuus*/
    switch (argc) {
        case 1:
            break;
        default:
            senderr("Annoit ohjelmalle liian monta argumenttiä. Yritä uudelleen!", E2BIG, argv);
    };
    /*mainin omat muuttujat*/
    pthread_t saikeet[SIZE];
    long kokonaisluvut[SIZE];
    
    /*täytetään lista kokonaisluvuilla*/
    for (int x = 0; x < SIZE; x++) {
        kokonaisluvut[x] = x;
    }

    /*säieistetään lukulista -> käytetään erillistä listaa argumenttien välittämisee jotta vältytää välitetävän tiedon vanhemiselta*/
    for (int osoitin = 0; osoitin < SIZE; osoitin++) {
        if (pthread_create(&saikeet[osoitin], NULL, thr_nsqrtf, (void*) &kokonaisluvut[osoitin]) != 0) {
            senderr("phtread_create epäonnistui!", errno, argv);
        }
    }
    /*varmistaa että kaikki säikeet ovat valmiita*/
    for (int osoitin = 0; osoitin < SIZE; osoitin++) {
        if (pthread_join(saikeet[osoitin], NULL) < 0) {
            senderr("pthread_join epäonnistui!", errno, argv);
        }
    }

    /*testaa ja tulostaa kokonaislukujen ja tulosten (neliöjuurien )perusteella lopputuloksen*/
    for (int osoitin = 0; osoitin < SIZE; osoitin++) {
        if (sqrtf( (float) osoitin ) != neliojuuret[osoitin]) { /* testaa onko neliojuuret listan arvo oikea ja tulostaa virheen jos ei vastaa odotettua arvoa */
            printf(" --VIRHE-- TULOS TULISI OLLA %f -> ", sqrtf( (float) kokonaisluvut[osoitin] ));
        }
        printf("Kokonaisluvun %ld neliöjuuri on: %f\n", kokonaisluvut[osoitin], neliojuuret[osoitin]);
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

/*ottaa argumenttina taulukon osoittimen osoittimen ja tallentaa laskutuloksen taulukkoon.*/
void *thr_nsqrtf (void *arg) {
    long x = *(long*) arg;
    neliojuuret[x] = sqrtf( (float) x );
    pthread_exit(NULL);
}
