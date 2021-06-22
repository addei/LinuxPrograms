#include "teht3.h"

//HUOM tietueiden määrä (limiitti) on staattinen ja muokattavissa teht3.h filussa.

int senderr(char *message, int errorcode, char *argv[]);

int main (int argc, char *argv[], char *env[]) {
    /*tarkistetaan argumenttien oikeellisuus*/
    switch (argc) {
        case 1:
            break;
        default:
            senderr("Annoit ohjelmalle liian monta argumenttiä. Yritä uudelleen!", E2BIG, argv);
    };

    char const *m_avain = "/shm_123456";
    char const *s_a_nimi = "/sem_a_987654";
    char const *s_b_nimi = "/sem_b_987654";

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
   
    /*pontteri yhteiskäyttöalueelle*/
    henkilo *p = (struct henkilo *) mmap(NULL, (sizeof(henkilo) * tietuelimiitti) + 1, PROT_READ, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        senderr("LUKIJA -> mmap: ongelma liittämisessä osoiteavaruuteen", errno, argv);
    }

    while (1) {
        henkilo *tietue = p + 1;
        sem_post(mysem_a);
        sem_wait(mysem_b);
        
        for (int x = 0; x < p[0].ika; x++) {
            
            if (p->ika < 0) {
                break;
            }

            /*tulostaa yhteiskäyttöalueen sisällön ja vapauttaa yhteismuistialueen*/
            printf("Tietue: %d\n", x + 1);
            printf("Henkilön nimi on: %s\n", tietue[x].nimi);
            printf("Henkilön ikä on: %d\n\n", tietue[x].ika);
            
        }

        if (p->ika < 0) {
            break;
        }
        sem_post(mysem_a);
    }
    
    /*sulje semaforit käytöstä ja ilmoittele A että voi jatkaa*/
    sem_close(mysem_b);
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
