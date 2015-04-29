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
        const char *p_brightness = "/sys/class/backlight/nv_backlight/brightness";
        FILE *f_brightness = NULL;
        char *c_brightness;
        long brightness = 0;

        if (argc != 2) {
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

        /*
        f_brightness = fopen(p_brightness, "rb");

        if (!f_brightness) {
                perror("fopen");
                exit(EXIT_FAILURE);
        }

        c_brightness = calloc(3, sizeof(char));

        size_t reading = fread(c_brightness, 1, 3, f_brightness);

        if (!reading) {
                perror("fread");
                exit(EXIT_FAILURE);
        }

        printf("Read in 3 chars from file, got content: %s (would set to %ld)\n", c_brightness, brightness); 
        free(c_brightness);
        c_brigthness = NULL;
        */
        f_brightness = fopen(p_brightness, "wb");

        if (!f_brightness) {
                perror("fopen");
                exit(EXIT_FAILURE);
        }

        //c_brightness = calloc(3, sizeof(char));
        c_brightness = argv[1];

        size_t writing = fwrite(c_brightness, 1, 3, f_brightness);

        if (!writing) {
                perror("fread");
                exit(EXIT_FAILURE);
        }

        printf("Wrote new brightness value %s!\n", c_brightness);
        //free(c_brightness);
        //c_brightness = NULL;

        /* WRITE VALUE INSIDE FILE at /sys/class/backlight/nv_backlight/brightness */
        
        if (*endptr != '\0'){        /* Not necessarily an error... */
                printf("Further characters after number: %s\n", endptr);
        }
        
        exit(EXIT_SUCCESS);
}
