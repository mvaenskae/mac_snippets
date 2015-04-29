#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct data {
        long brightness;
        char sign;
};

const char *p_brightness = "/sys/class/backlight/nv_backlight/brightness";

//void absolute_brightness();
//void relative_brightness(FILE, int pm, char *);
int process_input(int, char *, struct data *);
//void set_brightness();
//int get_brightness();

/*
 ** This code is based on the manpages on strtol. It removed an atoi() call
 ** and will just take 1 parameter. It will further check for the parameter
 ** to be within a certain range. If it does not match it will be set to the
 ** closest upper bound.
 */
int main(int argc, char *argv[])
{
        struct data *brgt = malloc(sizeof(brgt));
        memset(brgt, 0, sizeof(*brgt));

        if (argc != 2) {
                fprintf(stderr, "Usage: %s str\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        process_input(argc, argv[1], brgt);

        if (brgt->sign != '\0') {
        /* We have a relative value to set */
                printf("Relative value\n");
        } else {
        /* We have an absolute value */
                printf("Absolute value\n");
        }

        /* Convert value to string */

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
        /*f_brightness = fopen(p_brightness, "wb");

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
        */

        /* WRITE VALUE INSIDE FILE at /sys/class/backlight/nv_backlight/brightness */
        
        exit(EXIT_SUCCESS);
}

/*
 ** This function is responsible for reading in the stdin after it has been
 ** guaranteed to be of the correct format. It will parse the string and store
 ** extracted values within struct data before returning.
 */
int process_input(int argc, char *argv, struct data *store)
{
        const int base = 10;
        char *endptr, *str;
        long val = 0;

        str = argv;
        errno = 0;    /* To distinguish success/failure after call */
        val = strtol(str, &endptr, base);

        /* Check for various possible errors */
        if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
                        || (errno != 0 && val == 0)) {
                perror("strtol");
                exit(EXIT_FAILURE);
        }
        
        if (endptr == str) {
                fprintf(stderr, "No digits were found\n");
                exit(EXIT_FAILURE);
        }

        /* Check for existing sign to set flag for proper function call */
        if (argv[0] == '+') {
                store->sign = '+';
        } else if (argv[0] == '-') {
                store->sign = '-';
        }

        /* Ensure value is within range */
        if (val > 100) {
                val = 100;
        } else if (val < 1) {
                val = 1;
        }

        store->brightness = val;

        if (*endptr != '\0'){        /* Not necessarily an error... */
                printf("Further characters after number: %s\n", endptr);
        }
        
        return(EXIT_SUCCESS);
}
