#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern char *optarg;	/*argumentit*/
extern int optind;	/*argumenttien index*/

int main(int argc, char *argv[]) {

    if (argc == 1 ) {
        fprintf(stdout, "Usage: calc argument_value [-a n] [-s n] [-d n] [-m n]\n");
    }

    else {
        int c = 0, err = 0;
        float value; /*muuttuja tuloksen säilyttämistä varten*/

        value = atof (argv[1]); /*aloitusarvo*/

        while ((c = getopt (argc, argv, "a:s:d:m:")) != -1) {
            
            switch (c) {
                case 'a':
                    value += atoi(optarg);
                    break;
                case 's':
                    value -= atoi(optarg);
                    break;
                case 'd':
                    if (atoi(optarg) != 0) {
                        value /= atoi(optarg);
                    }
                    else {
                        err--;
                    }
                    break;
                case 'm':
                    value *= atoi(optarg);
                    break;
                case '?': //tunnistamaton arvo 
                    break;
            }
        }


        if (err == 0) {
            // Print the final value
            fprintf(stdout, "value : %.2f\n\n", value);
        }

        else {
            fprintf(stdout, "Not allowed to divide by zero\n");

        }
    }
    
    return 0;

}