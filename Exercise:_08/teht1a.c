#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/mman.h>
#include <errno.h>     /*makrot virheilmoituksia varten*/
#include <fcntl.h>
#include <semaphore.h>

#define _XOPEN_SOURCE 500
#define buffersize 4096 /*putken koko oletuksena*/

typedef struct henkilo {
    char nimi[80];
    int ika;
} henkilo;


int senderr(char *message, int errorcode, char *argv[]);

int main (int argc, char *argv[], char *env[]) {
    /*tarkistetaan argumenttien oikeellisuus*/
    switch (argc) {
        case 1:
            break;
        default:
            senderr("Annoit ohjelmalle liian monta argumenttiä. Yritä uudelleen!", E2BIG, argv);
    };

    char *m_avain = "/shm_123456";
    char *s_a_nimi = "/sem_a_987654";
    char *s_b_nimi = "/sem_b_987654";

    sem_t *mysem_a = sem_open(s_a_nimi, O_CREAT, 0600, 0);
    if (mysem_a == SEM_FAILED) {
        senderr("sem_open: semaforin luonti ei onnistu!", errno, argv);
    }

    sem_t *mysem_b = sem_open(s_b_nimi, O_CREAT, 0600, 0);
    if (mysem_b == SEM_FAILED) {
        senderr("sem_open: semaforin luonti ei onnistu!", errno, argv);
    }

    printf("Odottaa rinnakkaisprosessin käynnistymistä...\n");
    sem_wait(mysem_a);

    /* luodaan jaettu muistialue prosessille */
    int fd = shm_open(m_avain, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if(fd < 0) {
        senderr("Yhteisen muistialueen luonti epäonnistui!", errno, argv);
    }

    /*alustetaan muistialue*/
    int res = ftruncate( fd, sizeof(henkilo));
    if (res < 0) {
        senderr("Yhteisen muistialueen koon määrittäminen epäonnistui", errno, argv);
    }

    henkilo *p = (struct henkilo *) mmap(NULL, sizeof(henkilo), PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        senderr("EMO -> mmap: ongelma liittämisessä osoiteavaruuteen", errno, argv);
    }
    sem_post(mysem_b);
    
    /*lukee henkilön nimen stdinistä*/
    printf("Ole hyvä ja anna henkilön nimi!\n");
    while (1) {
        char buf_nimi[buffersize] = {'\0'};
        read(STDIN_FILENO, &buf_nimi, buffersize);
        if (strlen(buf_nimi) < 80) {
            /*korjaa \n painalluksen \0*/
            buf_nimi[(strlen(buf_nimi) -1)] = '\0';
            /*kopioidaan nimi puskurista yhteiskäyttöalueelle*/
            strcpy(p->nimi, buf_nimi);
            break;
        }
        printf("Ole hyvä ja anna henkilön nimi!\n");
    }
    
    /*puskuri ikä lukemista varten*/
    char buf_ika[buffersize] = {'\0'};
    /*otetaan ikä vastaan*/
    printf("Anna ikä!\n");
    /*kirjoittaa puskurista saadun tekstin tiedostoon.*/
    while (read(STDIN_FILENO, &buf_ika, buffersize)) {
        /*alustaa lipun*/
        int flag = 0;
        /*tarkistaa onko kaikki puskurista saadut arvot kokonaislukuja tai rivinvaihto*/
        for (int x = 0; x != strlen(buf_ika); x++) {
            if ( (isdigit(buf_ika[x]) | (buf_ika[x] == '\n') | (buf_ika[x] == '-') ) == 0) {
                printf("Anna syötteenä vain kokonaislukuja!\n");
                flag = -1;
                break;
            }
    
            /*tarkistaa löytyykö rivinvaihto -> jos löytyy nii break*/
            if ( (buf_ika[x] == '\n') ) {
                buf_ika[x] = '\0';
                break;
            }
        }
        /*jos lippu pysyy arvossaan -> kirjoita tiedostoon.*/
        if (flag == 0) {
            p->ika = atoi(buf_ika);
            break;
        }
    }

    //annetaan prosessi b ottaa yhteiskäyttöinen muisti käyttöön ja lukea sieltä dataa
    sem_post(mysem_a);
    //odotetaan että prosessi b antaa semafori b
    sem_wait(mysem_b);
    //siivous
    sem_close(mysem_a); /*sulje semafori a käytöstä*/
    sem_close(mysem_b); /*sulje semafori b käytöstä*/
    //poista semafori a ja b KJ:n kirjanpidosta
    sem_unlink(s_a_nimi);
    sem_unlink(s_b_nimi);
    
    /*vapautetaan tilavaraus keskusmuistista*/
    munmap(p, sizeof(henkilo));
    shm_unlink(m_avain);
    printf("KIRJOITTAJA: DONE!\n");
    exit(EXIT_SUCCESS);
    
}


/*erillinen funktio virheviesteille*/
int senderr(char *message, int errorcode, char *argv[]) {
    fprintf(stderr, "%s\n", message);
    errno = errorcode;
    perror(argv[0]);
    exit(EXIT_FAILURE);
}
