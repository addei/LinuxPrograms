#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>     /*makrot virheilmoituksia varten*/
#include <fcntl.h>
#include <signal.h>

#define _XOPEN_SOURCE 500
#define buffersize 4096 /*putken koko oletuksena*/

typedef struct henkilo {
    char nimi[80];
    int ika;
} henkilo;

int senderr(char *message, int errorcode, char *argv[]);
void signaalin_kasittelija(int sig, siginfo_t *info, void *ucontext);


int main (int argc, char *argv[], char *env[]) {
    /*tarkistetaan argumenttien oikeellisuus*/
    switch (argc) {
        case 1:
            break;
        default:
            senderr("Annoit ohjelmalle liian monta argumenttiä. Yritä uudelleen!", E2BIG, argv);
    };

    char *m_avain = "/shm_123456";
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

    /*signaalinkäsittelyyn tarvittavat asiat*/
    sigset_t signaali;
    sigemptyset(&signaali);

    //signaalin käsittely
    struct sigaction act;
    /*määritetään handleri*/
    act.sa_sigaction = signaalin_kasittelija;
    /*laitetaan handlerille flagi*/
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    /*määritetään signaalit toimimaan handlerin mukaisesti, ei talleneta vanhoja arvoja mihinkään*/
    sigaction(SIGUSR1, &act, NULL);


    /*lapsiprosessin luomiseen tarvittu muuttuja*/
    pid_t pid;
    pid = fork();

    /*lapsiprosessi jatkaa tästä*/
    if (pid == 0) {

        henkilo *p = (struct henkilo *) mmap(NULL, sizeof(henkilo), PROT_READ, MAP_SHARED, fd, 0);
        if (p == MAP_FAILED) {
            senderr("LAPSI -> mmap: ongelma liittämisessä osoiteavaruuteen", errno, argv);
        }

        /*laitetaan lapsiprosessi odottamaan SIGUSR1 signaalia emoprosessilta*/
        sigset_t mask;
        siginfo_t info;
        int signum;
        sigaddset(&mask, SIGUSR1);
        if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
            fprintf(stderr, "Cannot block SIGUSR1: %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
        //jää odottamaan signaalia SIGUSR1
        signum = sigwaitinfo(&mask, &info);
        if (signum == -1) {
            senderr("sigwaitinfo epäonnistui", errno, argv);
        }

        /*tulostaa yhteiskäyttöalueen sisällön ja vapauttaa yhteismuistialueen*/
        printf("Henkilön nimi on: %s\n", p->nimi);
        printf("Henkilön ikä on: %d\n", p->ika);
        /*vapautetaan tilanvaraus keskusmuistista*/
        munmap(p, sizeof(henkilo));
        shm_unlink(m_avain);

    }

    /*emoprosessi jatkaa tästä*/
    else {
        henkilo *p = (struct henkilo *) mmap(NULL, sizeof(henkilo), PROT_WRITE, MAP_SHARED, fd, 0);
        if (p == MAP_FAILED) {
            senderr("EMO -> mmap: ongelma liittämisessä osoiteavaruuteen", errno, argv);
        }
        
        /*lukee henkilön nimen stdinistä*/
        printf("Ole hyvä ja anna henkilön nimi! \n");
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
            printf("Ole hyvä ja anna henkilön nimi! \n");
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
        
        /*lähetetään SIGUSR1 signaali lapsiprosessille*/
        kill(pid, SIGUSR1);
        
        /*odotetaan että lapsiprosessi viimeistelee ajonsa*/
        wait(NULL);
        
        /*vapautetaan tilanvaraus keskusmuistista*/
        munmap(p, sizeof(henkilo));
        shm_unlink(m_avain);
        exit(EXIT_SUCCESS);
    }
}


/*erillinen funktio virheviesteille*/
int senderr(char *message, int errorcode, char *argv[]) {
    fprintf(stderr, "%s\n", message);
    errno = errorcode;
    perror(argv[0]);
    exit(EXIT_FAILURE);
}

/*signaalinkäsittelijä, ottaa parametrinä vastaanotetun signaalin tiedot.*/
void signaalin_kasittelija(int sig, siginfo_t *info, void *ucontext) {    
    char *signaali = "Tuli signaali SIGUSR1 (LAPSI)\n";
    switch (sig)
    {
    case SIGTERM:
        raise(SIGTERM);
        break;
    case SIGUSR1:
        write(STDOUT_FILENO, signaali, strlen(signaali));
        break;
    default:
        break;
    }
}
