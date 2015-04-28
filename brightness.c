#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>

/*
 ** This code is based on the manpages on strtol. It removed an atoi() call
 ** and will just take 1 parameter. It will further check for the parameter
 ** to be within a certain range. If it does not match it will be set to the
 ** closest upper bound.
 */
int main(int argc, char *argv[])
{
        const int base = 10;
        char *endptr, *str;
        const char *fp;
        long brightness = 0;

        if (argc > 2) {
                fprintf(stderr, "Usage: %s str\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        str = argv[1]; 
        errno = 0;    /* To distinguish success/failure after call */
        brightness = strtol(str, &endptr, base);
        
        /* Check for various possible errors */
        if ((errno == ERANGE &&
                        (brightness == LONG_MAX || brightness == LONG_MIN))
                        || (errno != 0 && brightness == 0)) {
                perror("strtol");
                exit(EXIT_FAILURE);
        }
        
        if (endptr == str) {
                fprintf(stderr, "No digits were found\n");
                exit(EXIT_FAILURE);
        }

        if (brightness > 100) {
                brightness = 100;
        } else if (brightness < 1) {
                brightness = 1;
        }

        /* WRITE VALUE INSIDE FILE at /sys/class/backlight/nv_backlight/brightness */
        
        if (*endptr != '\0'){        /* Not necessarily an error... */
                printf("Further characters after number: %s\n", endptr);
        }
        
        exit(EXIT_SUCCESS);
}
