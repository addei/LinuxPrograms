#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern char **__environ; /*ympäristömuuttujat*/

int main(int argc, char *argv[], char *envp[]) {

    if (argc < 2) { /*onko argumentteja annettu*/
        fprintf(stdout, "Ympäristömuuttujaa ei ole annettu!\n");
    }

    else {

        if (argc == 2) {

            if (getenv(argv[1]) == NULL) { /*Tarkistaa ettei etsittyä ympäristömuuttujaa löydy*/

                setenv(argv[1], "EI OLE ASETETTU", 0);
            
                for ( char **p = __environ; *p; p++) { /*tulosta lista for loopin ja pointterin pointterin avulla*/
                    fprintf(stdout, "%s\n", *p);
                }
            } 
        
            else { /* jos haettu ympäristömuuttuja löytyy */
                fprintf(stdout, "%s=%s\n", argv[1],getenv(argv[1])); /*tulostaa haetun ympäristömuuttujan nimen ja arvon */
            }
        }

        else { /* jos on liian monta argumenttia annettu*/
            fprintf(stdout, "Liian monta argumenttia annettu!\n");

        }
    }

    return 0;

}