#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../include/AMkd.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: executable [input filename] [output filename]\n");
        return EXIT_SUCCESS;
    }

    FILE *input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        fprintf(stderr, "Could not open \"%s\": %s", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (fseek(input_file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Could not seek the end of \"%s\": %s", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }
    int input_length = ftell(input_file);
    if (input_length == -1) {
        fprintf(stderr, "Could not tell the length of \"%s\": %s", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }
    rewind(input_file);
    if (ferror(input_file) != 0) {
        fprintf(stderr, "Could not rewind \"%s\": %s", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }
    char *input_string = (char *)malloc(sizeof(char) * (input_length + 1));
    if (input_string == NULL) {
        fprintf(stderr, "Could not allocate memory for input string: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (fread(input_string, 1, input_length, input_file) != input_length) {
        fprintf(stderr, "Could not read \"%s\" into memory: %s", argv[1], strerror(errno));
        free(input_string);
        exit(EXIT_FAILURE);
    }
    input_string[input_length] = '\0';
    if (fclose(input_file) == EOF) {
        fprintf(stderr, "Could not close \"%s\": %s", argv[1], strerror(errno));
        errno = 0;
    }

    AMkd_warning_flags warning_flags;
    AMkd_config input_encoding;
    AMkd_error_code return_code;
    if ((return_code = AMkd_detect_encoding(input_string, &input_encoding, &warning_flags)) != AMKD_ERROR_NONE) {
        free(input_string);
        if (return_code == AMKD_ERROR_MISSING_INPUT) {
            fprintf(stderr, "Expected input string, but got NULL.\n");
        }
        exit(EXIT_FAILURE);
    }

    char *output_string = NULL;

    if (input_encoding == AMKD_CONFIG_NONE) {
        if ((return_code = AMkd_encode(input_string, &output_string, AMKD_CONFIG_DEFAULT, &warning_flags)) != AMKD_ERROR_NONE) {
            fprintf(stderr, "An error has occured: %s\n", AMkd_get_error_string(return_code));
            fprintf(stderr, "errno state: %s\n", strerror(errno));
            if (warning_flags != AMKD_WARNING_NONE) {
                fprintf(stderr, "Warnings:\n");
                for (unsigned position = 0; position < 16; position++) {
                    if ((warning_flags & (1 << position)) != 0) {
                        fprintf(stderr, "  %s\n", AMkd_get_warning_string(1 << position));
                    }
                }
            }
            free(input_string);
            exit(EXIT_FAILURE);
        }
    } else {
        if ((return_code = AMkd_decode(input_string, &output_string, input_encoding, &warning_flags)) != AMKD_ERROR_NONE) {
            fprintf(stderr, "An error has occured: %s\n", AMkd_get_error_string(return_code));
            fprintf(stderr, "errno state: %s\n", strerror(errno));
            if (warning_flags != AMKD_WARNING_NONE) {
                fprintf(stderr, "Warnings:\n");
                for (unsigned position = 0; position < 16; position++) {
                    if ((warning_flags & (1 << position)) != 0) {
                        fprintf(stderr, "  %s\n", AMkd_get_warning_string(1 << position));
                    }
                }
            }
            free(input_string);
            exit(EXIT_FAILURE);
        }
    }
    free(input_string);

    int output_length = strlen(output_string);
    FILE *output_file = fopen(argv[2], "w");
    if (output_file == NULL) {
        fprintf(stderr, "Could not open \"%s\": %s", argv[2], strerror(errno));
        AMkd_deallocate_result(output_string);
        exit(EXIT_FAILURE);
    }
    if (fwrite(output_string, 1, output_length, output_file) != output_length) {
        fprintf(stderr, "Could not write to \"%s\": %s", argv[2], strerror(errno));
        AMkd_deallocate_result(output_string);
        exit(EXIT_FAILURE);
    }
    if (fclose(output_file) == EOF) {
        fprintf(stderr, "Could not close \"%s\": %s", argv[2], strerror(errno));
        errno = 0;
    }

    AMkd_deallocate_result(output_string);

    return EXIT_SUCCESS;
}
