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
#define max_number 999
#define minimum_number 0
#define nollaamisen_raja 3
#define MAX 2
#define MIN -2


//function declarations
int senderr(char *message, int errorcode, char *argv[]);

//thread declarations
void *thr_kasvattaja (void *arg);
void *thr_vähentäjä (void *arg);
void *thr_nollaaja (void *arg);

//globaalit muuttujat
int kokonaisluku = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main (int argc, char *argv[], char *env[]) {
    /*tarkistetaan argumenttien oikeellisuus*/
    switch (argc) {
        case 1:
            break;
        default:
            senderr("Annoit ohjelmalle liian monta argumenttiä. Yritä uudelleen!", E2BIG, argv);
    };
    //mainin omat muuttujat
    pthread_t saikeet[2][3] = {0}; /*taulukko sisältää tiedot säikeistä*/

    for (short i = 0; i <= 2; i++) {
        saikeet[0][i] = i;
    }

    printf("Ohjelma tulostaa 10x kertaa keskiarvon taulukon lukuarvoille!\n\n");

    //luodaan säie kasvattaja
    if (pthread_create(&saikeet[1][0], NULL, thr_kasvattaja, (void*) &saikeet[0][0]) != 0) {
        senderr("phtread_create epäonnistui!", errno, argv);
    }

    //luodaan säie vähentäjä
    if (pthread_create(&saikeet[1][1], NULL, thr_vähentäjä, (void*) &saikeet[0][1]) != 0) {
        senderr("phtread_create epäonnistui!", errno, argv);
    }

    //luodaan säie nollaaja
    if (pthread_create(&saikeet[1][2], NULL, thr_nollaaja, (void*) &saikeet[0][2]) != 0) {
        senderr("phtread_create epäonnistui!", errno, argv);
    }
 
    pthread_join(saikeet[1][2], NULL);
    
    //lopetetaan säikeet kasvattaja ja vähenttäjä
    pthread_cancel(saikeet[1][0]);
    pthread_cancel(saikeet[1][1]);


    printf("Säikeet sammutettu -> Ohjelman suoritus päättyy!\n");
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
void *thr_kasvattaja (void *arg) {
    char *argv[15] = {"thr_kasvattaja"};
    srand(time(0)); /*seed random generaattorille*/
    while (1) {
        if (pthread_mutex_lock(&mutex) < 0) { /*pidetään huoli että kasvattaja saa incrementtaa*/
            senderr("pthread_mutex_lock epäonnistui!", errno, argv);
        }
        kokonaisluku++;
        printf("kasvattaja: %d\n", kokonaisluku);
        if (pthread_mutex_unlock(&mutex) < 0) { /*vapautetaan lukko*/
            senderr("pthread_mutex_unlock epäonnistui!", errno, argv);
        }
        
        if (pthread_cond_signal(&cond) < 0) { /*laitetaan nollaaja tarkistamaa lukuarvo*/
            senderr("pthread_cond_signal epäonnistui!", errno, argv);
        }

        usleep((rand() % (max_number + 1 - minimum_number)) + minimum_number); /*odotetaan satunnaisesti mikrosekuntteja ylhäällä määritettyjen raja-arvojen perusteella*/
    }
    pthread_exit(NULL); /*tänne ei päästä koskaan ellei joku kosminen säteily flippaa bittiä muistissa*/
    
}

void *thr_vähentäjä (void *arg) {
    char *argv[17] = {"thr_vähentäjä"};
    srand(time(0)); /*seed random generaattorille*/
    while(1) {
        if (pthread_mutex_lock(&mutex) < 0) { /*pidetään huoli että arpoja arpoo luvut ensin*/
            senderr("pthread_mutex_lock epäonnistui!", errno, argv);
        }
        kokonaisluku--;
        printf("vähentäjä: %d\n", kokonaisluku);
        if (pthread_mutex_unlock(&mutex) < 0) { /*pidetään huoli että vähentäjä saa decrementtaa*/
            senderr("pthread_mutex_unlock epäonnistui!", errno, argv);
        }
        if (pthread_cond_signal(&cond) < 0) { /*laitetaan nollaaja tarkistamaa lukuarvo*/
            senderr("pthread_cond_signal epäonnistui!", errno, argv);
        }
        usleep((rand() % (max_number + 1 - minimum_number)) + minimum_number); /*odotetaan satunnaisesti mikrosekuntteja ylhäällä määritettyjen raja-arvojen perusteella*/
    }
    pthread_exit(NULL); /*tänne ei päästä koskaan ellei joku kosminen säteily flippaa bittiä muistissa*/
}

void *thr_nollaaja (void *arg) {
    char *argv[13] = {"thr_nollaaja"};
    int flag = 0;

    while (flag < nollaamisen_raja) { /*luuppi joka lopettaa suorittamisen kun ehdot täyttyy*/

        if (pthread_cond_wait(&cond,&mutex) < 0) { /*odotetaan sitä että kokonaisluku-muuttuja on muuttunut*/
            senderr("pthread_cond_wait epäonnistui!", errno, argv);
        }

        if ((kokonaisluku < MIN) | (kokonaisluku > MAX)) { /*tarkastetaan kokonaisluvun arvo ja nollataan se jos menee raja-arvojen yli*/
            kokonaisluku = 0;
            printf("nollaaja: luku nollataan!\n");
            flag++;
        }
    }
    
    printf("nollaaja: ajaminen päättyy!\n");
    pthread_exit(NULL);
}