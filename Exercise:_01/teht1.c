#include <stdio.h>
#include <unistd.h>
#include <string.h>


int main (int argc, char *argv[]) {
    fprintf(stdout, "Ohjelman nimi on %s\n", argv[0]);// OHJELMAN NIMI
    
    
    if (argc <= 1) { /*if -> ILMOITA ETTEI ARGUMENTTEJA OLE*/
        fprintf(stdout, "Et antanut komentorivillä yhtään argumenttia\n");

    }//KOMENTORIVIARGUMENTTI TESTI (if/else)
    
    else { /*else -> ILMOITA ARGUMENTTIEN MÄÄRÄ JA TULOSTA ARGUMENTIT*/
        fprintf(stdout, "Annoit ohjelman nimen perässä %d komentoriviargumenttia\n", (argc -1));
        for (int x = (argc - 1); x > - 1; x--) { /*for loop, käy listan läpi (index korjattu) ja tulostaa listan sisällön*/
            fprintf(stdout, "argv[%d] = %s\n", x, argv[x]);
        }
    }  
    
    return 0;
}