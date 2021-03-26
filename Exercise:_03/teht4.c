#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>  /*file control options*/
#include <unistd.h> /*POSIX standard symbolic constants and types*/

#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/stat.h>  /*käyttöoikeusbittimaskit*/
#include <dirent.h>    /*hakemistot*/
#include <errno.h>     /*makrot virheilmoituksia varten*/
#include <ctype.h>
#include <stdbool.h>   /*enabloi boolean datatyypin*/
#define buffersize 256 /*suurin koko tiedostolle on 255 (varaus 255+1)*/

/*virhetilanteen tulostus*/
int senderr(char *message, int errorcode, char *argv[]);

int main(int argc, char *argv[]) {
    
    /*tarkistetaan argumenttien oikeellisuus*/
    switch (argc) {
        case 1:
            senderr("Ohjelma tarvitsee kaksi hakemiston nimeä!", EPERM, argv);
        case 2:
            senderr("Ohjelma tarvitsee viel yhden hakemiston nimen!", EPERM, argv);
        case 3:
            break;
        default:
            senderr("Liian monta argumenttia annettu!", EAGAIN, argv);
    };

    /*hakemiston nimi on liian suuri*/
    if ( ( strlen(argv[1]) > 255 ) | ( strlen(argv[2]) > 255 ) ) {
        senderr("Hakemiston nimi on liian iso ollakseen hakemisto.", EAGAIN, argv);
    }

    /*luodaan muuttujat hakemistojen käsittelyä varten*/
    struct dirent **namelist_a;
    struct dirent **namelist_b;
	int a, b;

    /*luetaan hakemiston a tiedot puskuriin aakkosjärjestykseen ja otetaan ylös tiedostomäärä*/
	a = scandir(argv[1], &namelist_a, 0, alphasort);
	/*virheentarkistus*/
    if (a < 0)
	    senderr("Hakemiston lukemisvirhe!", errno, argv);
	else {
        /*luuppi käy hakemiston a listan läpi ja vertaa sitä hakemiston b sisältöön. Luuppi lisäksi tulostaa osumat sekä tiedoston tyypin (directory/regular/symlink)*/
	    while(a--) {
		    printf("\nEtsitään tiedostoa: \"%s\" hakemistosta\n", namelist_a[a]->d_name);
            int lippu = 0;
            /*luetaan hakemiston b tiedot puskuriin aakkosjärjestykseen ja otetaan ylös tiedostomäärä*/
            b = scandir(argv[2], &namelist_b, 0, alphasort);
            if (b < 0)
	            senderr("Hakemiston lukemisvirhe!", errno, argv);
            else {
                while(b--) {
                    /*verrataan hakemiston nimiä*/
                    if (strcmp (namelist_a[a]->d_name, namelist_b[b]->d_name) == 0) {
                        /*tulostetaan löydös*/
                        printf("Tiedosto \"%s\" löytyy molemmista hakemistoista\n", namelist_a[a]->d_name);
                        /*tiedostotyypin ja path määrittäminen*/
                        struct stat a_stats; /*luodaan rakenne tiedostotilastoja varte*/
                        struct stat b_stats; /*luodaan rakenne tiedostotilastoja varte*/
                        char a_path[(buffersize * 2)] = {'\0'};
                        sprintf(a_path, "%s/%s", argv[1], namelist_a[a]->d_name);
                        char b_path[(buffersize * 2)] = {'\0'};
                        sprintf(b_path, "%s/%s", argv[2], namelist_b[b]->d_name);
                        /*hakemiston a tiedoston tyypin tulostaminen*/
                        if ( lstat(a_path, &a_stats) != -1 ) {
                            
                            switch(a_stats.st_mode & S_IFMT) {
                                case S_IFREG: //is regular file
                                    printf("%s-dir: on tavallinen tiedosto (-)\n", argv[1]);
                                    break;
                                case S_IFLNK: //is symlink
                                    printf("%s-dir: on symlink (-)\n", argv[1]);
                                    break;
                                case S_IFDIR: //is directory
                                    printf("%s-dir: on hakemisto (-)\n", argv[1]);
                                    break;
                                default:
                                    printf("%s-dir: ei ole tavallinen tiedosto, symlink tai hakemisto\n", argv[1]);
                            };
                        }
                        /*hakemiston b tiedoston tyypin tulostaminen*/
                        if ( lstat(b_path, &b_stats) != -1 ) {
                            
                            switch(b_stats.st_mode & S_IFMT) {
                                case S_IFREG: //is regular file
                                    printf("%s-dir: on tavallinen tiedosto (-)\n", argv[2]);
                                    break;
                                case S_IFLNK: //is symlink
                                    printf("%s-dir: on symlink (-)\n", argv[2]);
                                    break;
                                case S_IFDIR: //is directory
                                    printf("%s-dir: on hakemisto (-)\n", argv[2]);
                                    break;
                                default:
                                    printf("%s-dir: ei ole tavallinen tiedosto, symlink tai hakemisto\n", argv[2]);
                            };
                        }
                        lippu++;
                        break;
                    }
		            free(namelist_b[b]);
	            }

                if ( lippu != 1 ) {
                    printf("Tiedostoa \"%s\" ei löytymyt hakemistosta \"%s\"\n", namelist_a[a]->d_name, "b");
                }
            }
            /*vapautetaan pointterit*/
            free(namelist_a[a]);
            free(namelist_b);
	    }
	    free(namelist_a);
   
	}

    /*palautetaan "EXIT_SUCCESS" suorituksen onnistumisen merkiksi*/
    exit(EXIT_SUCCESS);
}

/*erillinen funktio virheviesteille*/
int senderr(char *message, int errorcode, char *argv[]) {
    fprintf(stderr, "%s\n", message);
    errno = errorcode;
    perror(argv[0]);
    exit(EXIT_FAILURE);
}
