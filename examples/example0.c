#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../include/AMkd.h"

int main(int argc, char **argv)
{
    if (argc == 3) {
        FILE *input_file = fopen(argv[1], "rb");
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
        char *input_string = (char *)malloc(sizeof(char) * input_length);
        if (input_string == NULL) {
            fprintf(stderr, "Could not allocate memory for input string: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (fread(input_string, 1, input_length, input_file) != input_length) {
            fprintf(stderr, "Could not read \"%s\" into memory: %s", argv[1], strerror(errno));
            free(input_string);
            exit(EXIT_FAILURE);
        }
        if (fclose(input_file) == EOF) {
            fprintf(stderr, "Could not close \"%s\": %s", argv[1], strerror(errno));
            errno = 0;
        }

        AMkd_config input_encoding;
        AMkd_detect_encoding(input_string, &input_encoding);

        char *output_string = NULL;

        if (input_encoding.step_count == AMKD_CONFIG_NONE.step_count && input_encoding.letter == AMKD_CONFIG_NONE.letter) {
        #ifdef DEBUG
            printf(" Passed settings: %c:%d\n", AMKD_CONFIG_DEFAULT.letter, AMKD_CONFIG_DEFAULT.step_count);
        #endif // DEBUG
            if (AMkd_encode(input_string, &output_string, AMKD_CONFIG_DEFAULT) == AMKD_ERROR_OUT_OF_MEMORY) {
                fprintf(stderr, "Not enough memory for encoding string: %s", strerror(errno));
                free(input_string);
                exit(EXIT_FAILURE);
            }
        } else {
        #ifdef DEBUG
            printf(" Passed settings: %c:%d\n", input_encoding.letter, input_encoding.step_count);
        #endif // DEBUG
            if (AMkd_decode(input_string, &output_string, input_encoding) == AMKD_ERROR_OUT_OF_MEMORY) {
                fprintf(stderr, "Not enough memory for decoding string: %s", strerror(errno));
                free(input_string);
                exit(EXIT_FAILURE);
            }
        }
        free(input_string);

        int output_length = strlen(output_string);
        FILE *output_file = fopen(argv[2], "wb");
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
    } else {
        fprintf(stderr, "Usage: executable [input filename] [output filename]");
    }
    exit(EXIT_SUCCESS);
}
