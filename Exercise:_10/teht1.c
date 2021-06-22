//includes
#include <stdio.h> /*stdin - stdout*/
#include <stdlib.h> /*standard library definitions*/
#include <unistd.h> /*posix api*/
#include <math.h> /*C math library*/
#include <pthread.h> /*posix library for threading*/
#include <errno.h> /*error api*/
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>

//defines
#define SIZE 6
#define max_number 999
#define minimum_number 0
#define alkiot 10000
#define iteraatio 10


//function declarations
int senderr(char *message, int errorcode, char *argv[]);

//thread declarations
void *thr_arpoja (void *arg);
void *thr_tulostaja (void *arg);

//globaalit muuttujat
short taulukko[alkiot] = {0}; /*sisältää 10000 alkion arvot*/
pthread_rwlock_t lukko = PTHREAD_RWLOCK_INITIALIZER; /*RW-lukko*/


int main (int argc, char *argv[], char *env[]) {
    /*tarkistetaan argumenttien oikeellisuus*/
    switch (argc) {
        case 1:
            break;
        default:
            senderr("Annoit ohjelmalle liian monta argumenttiä. Yritä uudelleen!", E2BIG, argv);
    };
    //mainin omat muuttujat
    pthread_t saikeet[2][SIZE] = {0}; /*taulukko sisältää tiedot säikeistä*/

    for (short i = 0; i <= 5; i++) {
        saikeet[0][i] = i;
    }

    printf("Ohjelma tulostaa 10x kertaa keskiarvon taulukon lukuarvoille!\n\n");

    //luodaan säikeet arpoja ja tulostaja * 5
    if (pthread_create(&saikeet[1][0], NULL, thr_arpoja, (void*) &saikeet[0][0]) < 0) {
        senderr("phtread_create epäonnistui!", errno, argv);
    }

    /*luo 5 säiettä lukemaan dataa taulukosta*/
    for (short i = 1; i <= 5; i++) {
        if (pthread_create(&saikeet[1][i], NULL, thr_tulostaja, &saikeet[0][i]) < 0) {
            senderr("phtread_create epäonnistui!", errno, argv);
        }
    }

    /*varmistaa että kaikki säikeet ovat valmiita*/
    for (short i = 1; i <= 5; i++) {

        if (pthread_join(saikeet[1][i], NULL) < 0) {
            senderr("pthread_join epäonnistui!", errno, argv);
        }
    }
    printf("Säikeet 0 - 5 lopettivat toimintansa -> Ohjelman suoritus päättyy!\n");
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
void *thr_arpoja (void *arg) {
    char *argv[11] = {"thr_arpoja"};
    srand(time(0)); /*seed random generaattorille*/
    size_t length = sizeof(taulukko) / sizeof(taulukko[0]); /*otetaan ylös globaalin staattisen taulukon koko*/
    //arpoo taulukon arvot *iteraation osoittaman määrän kertaa
    for (short i = 0; i < iteraatio + 1; i++) {
        /*käy taulukon jokaisen elementin läpi ja kirjaa uuden lukuarvon taulukon sisälle*/  
        
        if (pthread_rwlock_wrlock(&lukko) < 0) {
            senderr("pthread_rwlock_wrlock epäonnistui!", errno, argv);
        }

        //printf("Arpoja arpoo luvut...\n");
        
        //switch case käytetään ohjaamaan toteutus ehdollisuutta. iteraatio (10 kerta) asettaa kaikkien alkioiden arvoksi -1
        switch (i)
        {
        case (iteraatio):
            for (size_t x = 0; x < length; x++) {
                taulukko[x] = -1;
            }
            break;
        
        default:
            for (size_t x = 0; x < length; x++) {
                taulukko[x] = (rand() % (max_number + 1 - minimum_number)) + minimum_number;
            }
            break;
        }
        
        //printf("Arpoja on valmis!\n");

        if (pthread_rwlock_unlock(&lukko) < 0) {
            senderr("pthread_rwlock_unlock epäonnistui!", errno, argv);
        }

        sleep(2);
    }
    pthread_exit(NULL);
    
}

void *thr_tulostaja (void *arg) {
    short thread = *(long*) arg;
    char *argv[15] = {"thr_tulostaja"};
    size_t length = sizeof(taulukko) / sizeof(taulukko[0]);
    /*lukee taulukon arvot kymmenen kertaa*/
    int i = 1;
    float etulos = 0;
    while (1) {
        int y = 0;

        if (pthread_rwlock_rdlock(&lukko) < 0) {
            senderr("pthread_rwlock_unlock epäonnistui!", errno, argv);
        }

        if (taulukko[alkiot -1] < 0) { /*jos taulukon viimeinen alkio on -1 luuppi lopettaa*/
            break;
        } 
        //printf("Tulostaja laskee luvut!\n");
        for (size_t x = 0; x < length; x++) { /*käy taulukon jokaisen elementin läpi ja tulostaa taulukon keskiarvon*/
           y = taulukko[x] + y;
        }
        //printf("Tulostaja on valmis!\n");

        if (pthread_rwlock_unlock(&lukko) < 0) {
            senderr("pthread_rwlock_unlock epäonnistui!", errno, argv);
        }

        if (etulos != ((float) y / (float) length)) {
            etulos = (float) y / (float) length;
            printf("Säie: %d Iteraation: %d Keskiarvo on: %f\n", thread, i, etulos);
            i++;
        }
        sleep(1);
    }
    pthread_exit(NULL);
}
