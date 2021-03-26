#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> /*file control options*/
#include <unistd.h> /*POSIX standard symbolic constants and types*/

#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/stat.h> /*käyttöoikeusbittimaskit*/
#include <errno.h> /**/

int main (int argc, char *argv[]) {

    if (argc == 1) {
        perror("Ole hyvä ja määritä tiedosto ensin!\n");
        exit(EXIT_FAILURE);
    }

    /*muuttuja tiedoston lukua varte*/
    int fd;

    /*avataan tiedostolukua varten*/
    if ((fd = open (argv[1], O_RDONLY)) < 0) {
        perror("Tiedoston avaaminen ei onnistunut.\n");
        exit(EXIT_FAILURE);
    }

    /*selvitetään tiedoston viimeisen tavun sijainti*/
    off_t size = lseek(fd, 0, SEEK_END);
    
    /*luodaan puskuri kahta tavua varten*/
    char buf[2] = {'\0'};
    
    /*käydään tiedosto läpi tavu tavulta nurinkurisesti arvot tulostaen*/
    while (size > -1) {
        pread(fd, buf, sizeof(char), size);
        if(buf[0] != '\0') {
            printf("%s", buf);
        }
        size--;
   }
    
    /*suljetaan tiedosto*/
    close(fd); 
    
    /*tulostetaan rivinväli jotta näyttäisi siistimmältä*/
    printf("\n");
    exit(EXIT_SUCCESS);
}