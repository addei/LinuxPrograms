#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/wait.h>
#include <errno.h>     /*makrot virheilmoituksia varten*/
#include <fcntl.h>

#define buffersize 80
#define _XOPEN_SOURCE 500

struct alkio {
    char *rivi_puskuri; /* pointteri dynaamisen muistin alkuun */
    ssize_t rivin_koko; /* dynaamisen muistialueen koko */
    struct alkio *ed; /* pointteri edelliseen alkioon */
};

int senderr(char *message, int errorcode, char *argv[]);
int push(struct alkio *x);
int pop(struct alkio *x);

/* globaali muuttuja */
struct alkio * pino = NULL; // Osoittaa pinon päällimäiseen alkioon 

int main (int argc, char *argv[], char *env[]) {

    /*tarkistetaan argumenttien oikeellisuus*/
    switch (argc) {
        case 2:
            break;
        default:
            senderr("Annoit ohjelmalle liian monta argumenttiä. Yritä uudelleen!", E2BIG, argv);
    };

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        senderr("Tiedoston avaaminen ei onnistunut.", errno, argv);
    }

    /* luodaan ja alustetaan alkio*/
    struct alkio i;
    i.rivi_puskuri = NULL;
    i.ed = NULL;

    /* määritellään alustava puskurin koko */
    size_t puskurin_koko = 10;

    /* varataan dynaaminen puskuri */
    i.rivi_puskuri = (char *) malloc (puskurin_koko * sizeof(char));
    if ( i.rivi_puskuri == NULL) {
        senderr("Puskurin allokointi epäonnistui", errno, argv);
    }
    
    /* luetaan eka rivi virrasta */
    i.rivin_koko = getline(&i.rivi_puskuri, &puskurin_koko, fp);
    /* viedään tiedoston sisältö */
    do {
        /*tulostetaan PUSH syöte -> helpoittaa tehtävän demonstrointia*/
        printf("--PUSH--\nRIVI: %sKOKO: %ld tavua\n\n", i.rivi_puskuri, i.rivin_koko);

        if (push(&i) < 0) {
            senderr("Virhe push funktion kanssa!", errno, argv);
        }

        /* määritellään alustava puskurin koko */
        size_t puskurin_koko = 1;
        
        /*luetaan rivi*/
        i.rivin_koko = getline(&i.rivi_puskuri, &puskurin_koko, fp);
        
        
    } while (i.rivin_koko != -1);

    /*suljetaan avattu tiedosto*/
    if ((fclose (fp)) < 0) {
        senderr("Tiedostokuvaajan sulkeminen ei onnistunut.", errno, argv);
    }

    /*vapautetaan dynaaminen allokointi ja siirretään pointteri osoittamaa NULL*/
    free(i.rivi_puskuri);
    i.rivi_puskuri = NULL;

    /*tyhjennetään luotu dynaaminen stack*/
    while((pop(&i) != -1)) {
        /*tulostetaan POP syöte -> helpoittaa tehtävän demonstrointia*/
        printf("--POP--\nRIVI: %sKOKO: %ld tavua\n\n", i.rivi_puskuri, i.rivin_koko);
        /*vapautetaan osoittimen osoittama muistipaikka muistista*/
        free(i.rivi_puskuri);
        i.rivi_puskuri = NULL;
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

int push(struct alkio *x) { //lisää pinoon alkion
    
    struct alkio *uusi;

    if ((uusi=(struct alkio *)malloc(sizeof(struct alkio))) == NULL) {
        perror("push: tilanvaraus uudelle alkiolle ei onnistunut.");
        return -1;
    }
    
    uusi->rivin_koko = x->rivin_koko;
    uusi->rivi_puskuri = (char *) malloc ((x->rivin_koko + 1) * sizeof(char));
    memcpy(uusi->rivi_puskuri, x->rivi_puskuri, x->rivin_koko);
    uusi->ed = pino;
    pino = uusi; // osoittaa taas päällimmäiseen alkioon
    return 0;
}

int pop(struct alkio *x) { // poista pinon päällimmäinen
    
    struct alkio *poistettava; // palauta arvo parametrissa
    
    if (pino == NULL) { // palauta -1, jos pino tyhjä
        return -1;
    }

    x->rivi_puskuri = (char *) malloc (( pino->rivin_koko ) * sizeof(char));
    memcpy(x->rivi_puskuri, pino->rivi_puskuri, pino->rivin_koko);
    x->rivin_koko = pino->rivin_koko;
    x->ed = pino->ed;
    poistettava = pino;
    pino = pino->ed;
    free(poistettava->rivi_puskuri);
    free(poistettava);
    return 0;
}
