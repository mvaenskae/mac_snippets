#define _DEFAULT_SOURCE
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct data {
        long val;
        int sign;
};

/* Read in maximum brightness via files, not hardcoded */
const static char *path_val = "/sys/class/leds/smc::kbd_backlight/brightness";
const static char *path_max_val = "/sys/class/leds/smc::kbd_backlight/max_brightness";

int process_input(char *, struct data *);
size_t set_value(FILE *, long);
long get_value(FILE *);

/*
 ** This code is based on the manpages on strtol. It removed an atoi() call
 ** and will just take 1 parameter. It will further check for the parameter
 ** to be within a certain range. If it does not match it will be set to the
 ** closest bound.
 */
int main(int argc, char *argv[])
{
        if (argc > 2) {
                fprintf(stderr, "Usage: %s str\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        struct data *value = malloc(sizeof(struct data));
        memset(value, 0, sizeof(struct data));
        FILE *fp = fopen(path_val, "rb+");

        if (!fp) {
                perror("fopen failed");
                exit(EXIT_FAILURE);
        }

        if (argc == 1) {
                printf("%ld\n", get_value(fp));
                if (fclose(fp)) {
                        perror("fclose failed");
                        exit(EXIT_FAILURE);
                }
                free(value);
                exit(EXIT_SUCCESS);
        }

        FILE *fp2 = fopen(path_max_val, "rb");

        if (!fp2) {
                perror("fopen failed");
                exit(EXIT_FAILURE);
        }

        process_input(argv[1], value);
        long val = get_value(fp);
        long max_val = get_value(fp2);
        long min_val = 0; /* set to 1 for lcd-backlight */

        if (fclose(fp2)) {
                perror("fclose failed");
                exit(EXIT_FAILURE);
        }

        /* We have a relative value to set */
        if (value->sign) {
                value->val = val + value->val;
        }

        /* Check and set sensible bounds */
        if (value->val > max_val) {
                value->val = max_val;
        } else if (value->val < min_val) {
                value->val = min_val;
        }

        size_t write = set_value(fp, value->val);
        
        if (fclose(fp)) {
                perror("fclose failed");
                exit(EXIT_FAILURE);
        }

        free(value);

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
        store->val = strtol(str, &endptr, base);

        /* Check for existing sign to set flag for proper function call */
        if (argv[0] == '+' || argv[0] == '-') {
                store->sign = 1;
        }

        /* Check for various possible errors */
        if ((errno == ERANGE 
                        && (store->val == LONG_MAX
                                || store->val == LONG_MIN))
                        || (errno != 0 && val == 0)) {
                perror("strtol");
                exit(EXIT_FAILURE);
        }
        
        if (endptr == str) {
                fprintf(stderr, "No digits were found\n");
                exit(EXIT_FAILURE);
        }

        if (*endptr != '\0'){        /* Not necessarily an error... */
                printf("Further characters after number: %s\n", endptr);
        }
        
        return(EXIT_SUCCESS);
}

/*
 ** This function returns the current value of the file.
 ** It will be read in as a long to return type-conforming values.
 ** It will NOT return the value of fread.
 */
long get_value(FILE *fp)
{
        /* Reading in the number and converting it to a long */
        char *temp = malloc(3 * sizeof(char));
        memset(temp, 0, 3*sizeof(char));

        size_t read = fread(temp, sizeof(char), 3, fp);
        long val = strtol(temp, NULL, 10);
        free(temp);

        return val;
}

/*
 ** This function is used just for setting the value. It reports errors
 ** according to errno of fwrite.
 **/
size_t set_value(FILE *fp, long val)
{
        /* Converting the long into a char[4], we need to include 0A */
        char *test = malloc(4*sizeof(char));
        memset(test, 0, 4*sizeof(char));
        int conv = snprintf(test, 4, "%ld", val);

        size_t write = fwrite(test, sizeof(char), 4, fp);
        free(test);

        return write;
}
