#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h> /*käyttötapalipukkeet*/
#include <sys/wait.h>
#include <errno.h>     /*makrot virheilmoituksia varten*/

int senderr(char *message, int errorcode, char *argv[]);

int main (int argc, char *argv[], char *env[]) {
    pid_t pid;

    /*luodaan lapsiprosessi*/
    if ((pid = fork()) < 0) {
        senderr("Forkkaus epäonnistui", errno, argv);
    } 

    /*lapsiprosessi alkaa tästä vikkelästi*/
    if (pid == 0) {
        printf("LAPSIPROSESSI -> Prosessi numero: pid=%d, Emoprosessin numero: pid=%d\n", getpid(), getppid());
        exit(EXIT_SUCCESS);
    }
    
    /*emoprosessi jatkaa tästä -> viivyttelee*/
    printf("EMOPROSESSI -> Prosessi numero: pid=%d, Emoprosessin numero: pid=%d\n", getpid(), getppid());
    /*nukkuu 10 sek*/
    sleep(10);
    /*odota lapsiprosessin päättymistä -> sulje prosessi*/
    waitpid(pid, NULL, 0);
    exit(EXIT_SUCCESS);
}

//HUOM EMOPROSESSIN EMOPROSESSI ON KOMENTOTULKKI PID -> BASH/ZSH/FISH
//LAPSIPROSESSI NÄKYY PROSESSILISTALLA ZOMBINA 

/*erillinen funktio virheviesteille*/
int senderr(char *message, int errorcode, char *argv[]) {
    fprintf(stderr, "%s\n", message);
    errno = errorcode;
    perror(argv[0]);
    exit(EXIT_FAILURE);
}
