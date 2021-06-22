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

//defines
#define SIZE 2


//function declarations
int senderr(char *message, int errorcode, char *argv[]);
int getintfromuser();

//thread declarations
void *thr_kertoma (void *arg);
void *thr_summa (void *arg);

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

    /*kaapataan opastetusti kokonaisluku käyttäjältä*/
    printf("Anna kokonaisluku!\n");
    int x = getintfromuser();

    /*säieistetään lukulista -> käytetään erillistä listaa argumenttien välittämisee jotta vältytää välitetävän tiedon vanhemiselta*/
    /*suurin kertoma minkä pystyy esittää on 12!*/
    if (pthread_create(&saikeet[0], NULL, thr_kertoma, (void*) &x) < 0) {
        senderr("phtread_create epäonnistui!", errno, argv);
    }

    if (pthread_create(&saikeet[1], NULL, thr_summa, (void*) &x) < 0) {
        senderr("phtread_create epäonnistui!", errno, argv);
    }
    /*luodaan pointteri osoittamaan toisen säikeen luomaan muistialueeseen*/
    void *kertooma, *summa;
    /*varmistaa että kaikki säikeet ovat valmiita ja otetaan koppi säikeen return pointterista*/

    if (pthread_join(saikeet[0], &kertooma) < 0) {
        senderr("pthread_join epäonnistui!", errno, argv);
    }

    if (pthread_join(saikeet[1], &summa) < 0) {
        senderr("pthread_join epäonnistui!", errno, argv);
    }

    /*tulostaa laskutoimitukset*/
    printf("Kertooma: %d\nSumma: %d\n", *(int *) kertooma, *(int *) summa);
    /*vapautetaan dynaamiset muistialueet*/
    free(kertooma);
    free(summa);
    exit(EXIT_SUCCESS);

}

/*erillinen funktio virheviesteille*/
int senderr(char *message, int errorcode, char *argv[]) {
    fprintf(stderr, "%s\n", message);
    errno = errorcode;
    perror(argv[0]);
    exit(EXIT_FAILURE);
}

int getintfromuser() { //ottaa käyttäjältä syötteen ja palauttaa sen kokonaislukuna 
    #define buffersize 4096

    char buf_int[buffersize] = {'\0'};

    /*kirjoittaa puskurista saadun tekstin tiedostoon.*/
    while (read(STDIN_FILENO, &buf_int, buffersize)) {
        /*alustaa lipun*/
        int flag = 0;
        /*tarkistaa onko kaikki puskurista saadut arvot kokonaislukuja tai rivinvaihto*/
        for (int x = 0; x != strlen(buf_int); x++) {
            if ( (isdigit(buf_int[x]) | (buf_int[x] == '\n') | (buf_int[x] != '-') ) == 0) {
                printf("Anna syötteenä vain positiivisia kokonaislukuja!\n");
                flag = -1;
                break;
            }
    
            /*tarkistaa löytyykö rivinvaihto -> jos löytyy nii break*/
            if ( (buf_int[x] == '\n') ) {
                buf_int[x] = '\0';
                break;
            }
        }
        /*jos lippu pysyy arvossaan -> kirjoita tiedostoon.*/
        if (flag == 0) {
            break;
        }
    }
    return atoi(buf_int); //turvallinen sillä palauttaa 0 jos muunnosta ei voi tehdä
}


/*ottaa argumenttina taulukon osoittimen osoittimen ja tallentaa laskutuloksen taulukkoon.*/
void *thr_kertoma (void *arg) {
    int x = *(long*) arg;
    int *returnvalue = (int *)malloc(sizeof(int));
    
    if (returnvalue == NULL) { //jos malloc epäonnistuu
        pthread_exit(NULL);
    }
    
    *returnvalue = 1;
    for (int i = 1; i < x + 1; i++) {
        *returnvalue = *returnvalue * i;
    }
    pthread_exit((void *) returnvalue);
    
}

void *thr_summa (void *arg) {
    int x = *(long*) arg;
    int *returnvalue = (int *)malloc(sizeof(int));

    if (returnvalue == NULL) { //jos malloc epäonnistuu
        pthread_exit(NULL);
    }
    
    for (int i = 1; i < x + 1; i++) {
        *returnvalue = *returnvalue + i;
    }
    pthread_exit((void *) returnvalue);
    
}
