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
#define tietuelimiitti 5
#define nimilimiitti 80

typedef struct henkilo {
    char nimi[nimilimiitti];
    int ika;
} henkilo;
