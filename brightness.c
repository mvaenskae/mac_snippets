#define _DEFAULT_SOURCE
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct data {
        long brightness;
        int sign;
};

const char *p_brightness = "/sys/class/backlight/nv_backlight/brightness";

int process_input(char *, struct data *);
size_t set_brightness(FILE *, const long *);
long get_brightness(FILE *);

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
        FILE *fp = NULL;

        if (argc != 2) {
                fprintf(stderr, "Usage: %s str\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        long temp_brightness = get_brightness(fp);

        process_input(argv[1], brgt);

        if (brgt->sign) {
        /* We have a relative value to set */
                brgt->brightness = get_brightness(fp) + brgt->brightness;
        }

        /* Check and set sensible bounds */
        if (brgt->brightness > 100) {
                brgt->brightness = 100;
        } else if (brgt->brightness < 1) {
                brgt->brightness = 1;
        }

        printf("We would set brightness to %ld from %ld\n", brgt->brightness, temp_brightness);

        free(brgt);

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
int process_input(char *argv, struct data *store)
{
        const int base = 10;
        char *endptr, *str;
        long val = 0;

        str = argv;
        errno = 0;    /* To distinguish success/failure after call */
        store->brightness = strtol(str, &endptr, base);

        /* Check for various possible errors */
        if ((errno == ERANGE 
                        && (store->brightness == LONG_MAX
                                || store->brightness == LONG_MIN))
                        || (errno != 0 && val == 0)) {
                perror("strtol");
                exit(EXIT_FAILURE);
        }
        
        if (endptr == str) {
                fprintf(stderr, "No digits were found\n");
                exit(EXIT_FAILURE);
        }

        /* Check for existing sign to set flag for proper function call */
        if (argv[0] == '+' || argv[0] == '-') {
                store->sign = 1;
        }
        // handle error!

        if (*endptr != '\0'){        /* Not necessarily an error... */
                printf("Further characters after number: %s\n", endptr);
        }
        
        return(EXIT_SUCCESS);
}

/*
 ** This function returns the current brightness of the file.
 ** It will be read in as a long to return type-conforming values.
 ** It will NOT return the value of fread.
 */
long get_brightness(FILE *fp)
{
        if (fp) {
                perror("File still open. Closing file as a precaution!i");
                if (fclose(fp)) {
                        perror("fclose failed");
                        exit(EXIT_FAILURE);
                }
                
        }

        fp = fopen(p_brightness, "rb");

        if (!fp) {
                perror("fopen failed");
                exit(EXIT_FAILURE);
        }

        /* Reading in the number and converting it to a long */
        char *temp = malloc(3 * sizeof(char));
        memset(temp, 0, 3*sizeof(char));
        size_t read = fread(temp, sizeof(char), 3, fp);
        long val = strtol(temp, NULL, 10);
        free(temp);

        if (fclose(fp)) {
                perror("fclose failed");
                exit(EXIT_FAILURE);
        }

        return val;
}

/*
 ** This function is used just for setting the brightness. It reports errors
 ** according to errno of fwrite.
 **/
size_t set_brightness(FILE *fp, const long *val)
{
        size_t write;

        if (fp) {
                perror("File still open. Closing file as a precaution!i");
                if (fclose(fp)) {
                        perror("fclose failed");
                        exit(EXIT_FAILURE);
                }
                
        }

        fp = fopen(p_brightness, "wb");

        if (!fp) {
                perror("fopen failed");
                exit(EXIT_FAILURE);
        }

        write = fwrite(val, sizeof(long), 1, fp);

        if (fclose(fp)) {
                perror("fclose failed");
                exit(EXIT_FAILURE);
        }

        return write;
}
