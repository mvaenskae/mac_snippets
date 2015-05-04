#define _DEFAULT_SOURCE
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct data {
        long value;
        int sign;
};

/* Read in maximum brightness via files, not hardcoded */
const static char *path_val = "/sys/devices/platform/applesmc.768/fan1_output";
const static char *path_max_val = "/sys/devices/platform/applesmc.768/fan1_max";
const static char *path_min_val = "/sys/devices/platform/applesmc.768/fan1_min";
const static char *path_control = "/sys/devices/platform/applesmc.768/fan1_manual";

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
        if (argc != 2) {
                fprintf(stderr, "Usage: %s A(utomatic)/{NUMBER}\n", argv[0]);
                exit(EXIT_FAILURE);
        }

        /* 
         ** We set back to automatic, no need to set anything else except one
         ** file.
         */
        if (!strncmp(argv[1], "A", 1)) {
                FILE *fp_control = fopen(path_control, "rb+");

                if (!fp_control) {
                        perror("fopen failed");
                        exit(EXIT_FAILURE);
                }

                set_value(fp_control, 0);

                if (fclose(fp_control)) {
                        perror ("fclose failed");
                        exit(EXIT_FAILURE);
                }

                exit(EXIT_SUCCESS);
        }

        struct data *number = malloc(sizeof(struct data));
        memset(number, 0, sizeof(struct data));

        FILE *fp_val = fopen(path_val, "rb+");
        if (!fp_val) {
                perror("fopen failed");
                exit(EXIT_FAILURE);
        }

        FILE *fp_max = fopen(path_max_val, "rb");
        if (!fp_max) {
                perror("fopen failed");
                exit(EXIT_FAILURE);
        }

        FILE *fp_min = fopen(path_min_val, "rb");
        if (!fp_min) {
                perror("fopen failed");
                exit(EXIT_FAILURE);
        }

        FILE *fp_control = fopen(path_control, "rb+");
        if (!fp_control) {
                perror("fopen failed");
                exit(EXIT_FAILURE);
        }

        process_input(argv[1], number);
        long val = get_value(fp_val);
        long max_val = get_value(fp_max);
        long min_val = get_value(fp_min);

        if (fclose(fp_max)) {
                perror("fclose failed");
                exit(EXIT_FAILURE);
        }

        if (fclose(fp_min)) {
                perror("fclose failed");
                exit(EXIT_FAILURE);
        }

        /* We have a relative value to set */
        if (number->sign) {
                number->value = val + number->value;
        }

        /* Check and set sensible bounds */
        if (number->value > max_val) {
               number->value = max_val;
        } else if (number->value < min_val) {
                number->value = min_val;
        }

        size_t write_control = set_value(fp_control, 1L);
        size_t write_val = set_value(fp_val, number->value);

        if (fclose(fp_control)) {
                perror("fclose failed");
                exit(EXIT_FAILURE);
        }
        
        if (fclose(fp_val)) {
                perror("fclose failed");
                exit(EXIT_FAILURE);
        }

        free(number);

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
        store->value = strtol(str, &endptr, base);

        /* Check for existing sign to set flag for proper function call */
        if (argv[0] == '+' || argv[0] == '-') {
                store->sign = 1;
        }

        /* Check for various possible errors */
        if ((errno == ERANGE 
                        && (store->value == LONG_MAX
                                || store->value == LONG_MIN))
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
 ** This function returns the current value of the file.  It will be read in as
 ** a long to return type-conforming values.  It will NOT return the value of
 ** fread.
 */
long get_value(FILE *fp)
{
        /* Reading in the number and converting it to a long */
        char *tmp = malloc(20 * sizeof(char));
        memset(tmp, 0, 20*sizeof(char));

        size_t read = fread(tmp, sizeof(char), 20, fp);
        long val = strtol(tmp, NULL, 10);
        free(tmp);

        return val;
}

/*
 ** This function is used just for setting the value. It reports errors
 ** according to errno of fwrite.
 */
size_t set_value(FILE *fp, long val)
{
        /* Converting the long into a char[20] */
        char *tmp = malloc(20*sizeof(char));
        memset(tmp, 0, 20*sizeof(char));
        int conv = snprintf(tmp, 20, "%ld", val);

        size_t write = fwrite(tmp, sizeof(char), 20, fp);
        free(tmp);

        return write;
}
