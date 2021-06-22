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

    /*odotetaan lupaa jatkaa prosessi A:lta*/
    sem_wait(mysem_b);
    

    /* luodaan jaettu muistialue prosessille */
    int fd = shm_open(m_avain, O_RDONLY, 0600);
    if(fd < 0) {
        senderr("Yhteisen muistialueen luonti epäonnistui!", errno, argv);
    }
   
    henkilo *p = (struct henkilo *) mmap(NULL, sizeof(henkilo), PROT_READ, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        senderr("LUKIJA -> mmap: ongelma liittämisessä osoiteavaruuteen", errno, argv);
    }

    /*ilmoitetaan prosessi A että alustus on tehty loppuun ja ollaan valmiita odottamaan syötteitä*/
    sem_post(mysem_a);

    while (1) {
        sem_wait(mysem_b);
        if (p->ika < 0) {
            break;
        }
        /*tulostaa yhteiskäyttöalueen sisällön ja vapauttaa yhteismuistialueen*/
        printf("Henkilön nimi on: %s\n", p->nimi);
        printf("Henkilön ikä on: %d\n\n", p->ika);
        sem_post(mysem_a);
    }
    
    /*sulje semaforit käytöstä ja ilmoittele A että voi jatkaa*/
    sem_close(mysem_b);
    sem_post(mysem_a);
    sem_close(mysem_a); 
    /*vapautetaan tilavaraus keskusmuistista*/
    munmap(p, sizeof(henkilo));
    shm_unlink(m_avain);
    printf("LUKIJA: DONE!\n");
    exit(EXIT_SUCCESS);
    
}


/*erillinen funktio virheviesteille*/
int senderr(char *message, int errorcode, char *argv[]) {
    fprintf(stderr, "%s\n", message);
    errno = errorcode;
    perror(argv[0]);
    exit(EXIT_FAILURE);
}
