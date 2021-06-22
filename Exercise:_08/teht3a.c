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
    const char *ohje ="Syötä henkilötiedot -> tiedot näkyvät kumppaniprosessissa.\nTulostaaksesi arvot, paina p + enter\nPoistuaksesi syötteestä, paina q + enter\n\n";

    /*luodaan kaksi semaforia*/
    sem_t *mysem_a = sem_open(s_a_nimi, O_CREAT, 0600, 0);
    if (mysem_a == SEM_FAILED) {
        senderr("sem_open: semaforin luonti ei onnistu!", errno, argv);
    }

    sem_t *mysem_b = sem_open(s_b_nimi, O_CREAT, 0600, 0);
    if (mysem_b == SEM_FAILED) {
        senderr("sem_open: semaforin luonti ei onnistu!", errno, argv);
    }
    
    /* luodaan jaettu muistialue prosessille */
    int fd = shm_open(m_avain, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if(fd < 0) {
        senderr("Yhteisen muistialueen luonti epäonnistui!", errno, argv);
    }
    
    /*alustetaan muistialue*/
    int res = ftruncate( fd, (sizeof(henkilo) * tietuelimiitti) + 1);
    if (res < 0) {
        senderr("Yhteisen muistialueen koon määrittäminen epäonnistui", errno, argv);
    }

    henkilo *p = (struct henkilo *) mmap(NULL, (sizeof(henkilo) * tietuelimiitti) + 1, PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        senderr("EMO -> mmap: ongelma liittämisessä osoiteavaruuteen", errno, argv);
    }
    
    //annetaan prosessi B:n ottaa yhteisalue käyttöön.
    sem_post(mysem_b);

    /*tulostetaan ohjeet ohjelman käyttöön*/
    printf(ohje);
    p->ika = 0; /*nollaa metadatana käytetyn tietueiden lukumäärän osoittajan*/

    
    while (1) {
        henkilo *tietue = p + 1;/*luodaan pointteri joka osoittaa metadatasta (p) seuraavaan soluun*/
        int lippu = 1; /*käytetään komentojen ohjaamiseen, p -> asettaa arvoksi 0 = lopettaa syötteiden vastaanottamisen ja laittaa kumppaniprosessin tulostaa tietueet, q asettaa arvoksi -1 ja lopettaa ohjelman suorituksen*/
        /*kerätään tietueita ja tallennetaan sisältö jaetulle puskureiden kautta alueelle*/
        for (int x = 0; x < tietuelimiitti; x++) {
            char puskuri_nimi[buffersize] = {'\0'};
            int puskuri_ika;
            /*lukee henkilön nimen stdinistä*/
            printf("Ole hyvä ja anna henkilön nimi!\n");
            while (1) {
                char buf_nimi[buffersize] = {'\0'};
                read(STDIN_FILENO, &buf_nimi, buffersize);

                /*lopetetaan prosessin ja kumppaniprosessin ajo hallitusti*/
                if ( (buf_nimi[0] == 'q') | (buf_nimi[0] == 'Q') ) {
                    if (buf_nimi[1] == '\n') {
                        lippu = -1;
                    }
                }

                /*lopetetaan syötteen otto ja annetaan prosessi B lukea tietoa yhteiseltä alueelta*/
                if ( (buf_nimi[0] == 'p') | (buf_nimi[0] == 'P') ) {
                    if (buf_nimi[1] == '\n') {
                        lippu = 0;
                        break;
                    }
                }

                /*varmistetaan että stdinistä saatu data on sallitun kokoinen*/
                if (strlen(buf_nimi) < nimilimiitti) {
                    /*korjaa \n painalluksen \0*/
                    buf_nimi[(strlen(buf_nimi) -1)] = '\0';
                    /*kopioidaan nimi puskurista yhteiskäyttöalueelle*/
                    strcpy(puskuri_nimi, buf_nimi);
                    break;
                }
                printf(ohje);
            }

            /*katkasee syötteen oton ja antaa kumppaniprosessin jaktaa*/
            if (lippu == 0) {
                break;
            }

            /*katkasee syötteen oton, asettaa metadata tietueeseen merkin ohjelman lopetuksesta*/
            if (lippu == -1) {
                p->ika = -1;
                break;
            }
        
            /*otetaan ikä vastaan*/
            printf("Anna ikä!\n");
            /*kirjoittaa puskurista saadun tekstin tiedostoon.*/
            while (lippu) {
                /*puskuri ikä lukemista varten*/
                char buf_ika[buffersize] = {'\0'};
                read(STDIN_FILENO, &buf_ika, buffersize);
                /*alustaa lipun*/
                int flag = 0;
                /*tarkistaa onko kaikki puskurista saadut arvot kokonaislukuja tai rivinvaihto*/
                for (int y = 0; y != strlen(buf_ika); y++) {
                    if ( (isdigit(buf_ika[y]) | (buf_ika[y] == '\n') | (buf_ika[y] == '-') ) == 0) {
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
                    puskuri_ika = atoi(buf_ika);
                    printf("\n");
                    break;
                }
            }

            /*lisää kerätyt tiedot yhteiskäyttöalueella lymyilevään tietueeseen*/
            if (lippu == 1) {
                /*muutetaan yhteisalueella lymyilevät arvot*/
                strcpy(tietue[x].nimi, puskuri_nimi);
                tietue[x].ika = puskuri_ika;
                p->ika++;
            }
        }
        /*lopetetaa luuppaus ja siivotaan jäljet + suljetaan ohjelma*/
        if (lippu == -1) {
            sem_post(mysem_b);
            break;
        }

        /*odotetaan että prosessi B on valmis*/
        sem_wait(mysem_a);
        printf("Tulostetaan kumppaniprosessiin %d kpl henkilötietoa\n\n", p->ika);
        /*ilmoitetaan B prosessille että voi nyt käsitellä yhteisen alueen tietoja*/
        sem_post(mysem_b);
        sem_wait(mysem_a); /*odottaa että kumppaniprosessi on käynyt yhteisen alueen tiedot läpi*/
        p->ika = 0; /*nollaa metadatana käytetyn tietueiden lukumäärän osoittajan*/

    }

    //siivous
    sem_close(mysem_b); /*sulje semafori b käytöstä*/
    sem_close(mysem_a); /*sulje semafori a käytöstä*/
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

