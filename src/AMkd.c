#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../include/AMkd.h"

const AMkd_config AMKD_CONFIG_DEFAULT = 6;
const AMkd_config AMKD_CONFIG_NONE = 0;

const char *AMKD_LINEBREAK_ENCODED = "<E>";
const char *AMKD_LINEBREAK_DECODED = "\n";

static unsigned AMkd_calculate_size(const char *input_str, AMkd_config input_config, AMkd_config output_config)
{
#ifdef DEBUG
    printf(" Calculating size...\n");
#endif  // DEBUG
    unsigned input_length = strlen(input_str) + 1;

    if (input_config == output_config) {
        return input_length;
    }

    int header_length_diff = 0;
    const char *sought_sequence;
    const char *replace_sequence;

    if (output_config == AMKD_CONFIG_NONE) {
        // for calculating the size after decoding
        sscanf(input_str, "{<%*c:%*d>}\n%n", &header_length_diff);
        header_length_diff *= -1;
        sought_sequence = AMKD_LINEBREAK_ENCODED;
        replace_sequence = AMKD_LINEBREAK_DECODED;
    } else {
        // for calculating the size after encoding
        header_length_diff = snprintf(NULL, 0, "{<C:%d>}\n", output_config);
        sought_sequence = AMKD_LINEBREAK_DECODED;
        replace_sequence = AMKD_LINEBREAK_ENCODED;
    }

    int linebreak_count = 0;
    const char *curr_linebreak = strstr(input_str, sought_sequence);
    while (curr_linebreak != NULL) {
        linebreak_count++;
        curr_linebreak = strstr(curr_linebreak + 1, sought_sequence);
    }

    int additional_decoded_linebreaks = 0;
    if (input_config == AMKD_CONFIG_NONE) {
        // taking into account "plain" (decoded) line breaks in encoded string
        // that are traditionally added every `output_config` encoded line breaks
        // and on the end of file
        additional_decoded_linebreaks = linebreak_count / output_config + 1;
    }

#ifdef DEBUG
    printf(" variables:\n");
    printf("  input_length: %d\n", input_length);
    printf("  linebreak_count: %d\n", linebreak_count);
    printf("  strlen(replace_sequence): %u\n", strlen(replace_sequence));
    printf("  strlen(sought_sequence): %u\n", strlen(sought_sequence));
    printf("  output_config: %d\n", output_config);
    printf("  header_length_diff: %d\n", header_length_diff);
    printf("  additional_decoded_linebreaks: %d\n", additional_decoded_linebreaks);
#endif  // DEBUG

    return (input_length + linebreak_count * (strlen(replace_sequence) - strlen(sought_sequence))
        + additional_decoded_linebreaks * strlen(AMKD_LINEBREAK_DECODED) + header_length_diff);
}

AMkd_error_code AMkd_decode(const char *encoded_str, char **decoded_str, AMkd_config config, AMkd_warning_flags *warning_flags)
{
#ifdef DEBUG
    printf("Decoding...\n");
#endif  // DEBUG
    if (warning_flags != NULL) {
        *warning_flags = AMKD_WARNING_NONE;
    }
    if (encoded_str == NULL) {
        return AMKD_ERROR_MISSING_INPUT;
    }

    AMkd_config config_from_header = AMKD_CONFIG_NONE;
    AMkd_error_code inner_error;
    AMkd_warning_flags inner_warnings;
    if ((inner_error = AMkd_detect_encoding(encoded_str, &config_from_header, &inner_warnings)) != AMKD_ERROR_NONE) {
        return inner_error;
    }
    if (warning_flags != NULL) {
        *warning_flags |= inner_warnings;
    }
    if (config == AMKD_CONFIG_NONE) {
        // use config read from header as there is no user-provided one
        if (config_from_header == AMKD_CONFIG_NONE) {
            return AMKD_ERROR_MISSING_CONFIG;
        }
        config = config_from_header;
    } else {
        // config read from header is overriden by user-provided one
        if (warning_flags != NULL && config_from_header != AMKD_CONFIG_NONE) {
            *warning_flags |= AMKD_WARNING_SURPLUS_CONFIG;
        }
    }
#ifdef DEBUG
    printf(" Config: %d\n", config);
#endif  // DEBUG

    unsigned encoded_length = strlen(encoded_str),
        encoded_index = 0,
        decoded_index = 0,
        step = 1,
        decoded_str_max_size = AMkd_calculate_size(encoded_str, config, AMKD_CONFIG_NONE);
#ifdef DEBUG
    printf(" Max decoded_str size: %d\n", decoded_str_max_size);
#endif  // DEBUG
    *decoded_str = (char *)malloc((decoded_str_max_size + 1) * sizeof(char));
    if (*decoded_str == NULL) {
        return AMKD_ERROR_OUT_OF_MEMORY;
    }

    unsigned header_length = 0;
    sscanf(encoded_str, "{<%*c:%*d>}\n%n", &header_length);
    encoded_index += header_length;

    while (encoded_index < encoded_length) {
        if (step > config) {
            step = 1;
        }
        int shift = (step + 1) / 2;
        if (step % 2 == 1) {
            shift *= -1;
        }

        if (encoded_index + strlen(AMKD_LINEBREAK_DECODED) - 1 <= encoded_length
                && !strncmp(encoded_str + encoded_index, AMKD_LINEBREAK_DECODED, strlen(AMKD_LINEBREAK_DECODED))) {
        #ifdef VERBOSE_DEBUG
            printf(" Skipping: '%s' (step: %d, shift: %d). Position: in %d/%d, out %d/%d\n", 
                AMKD_LINEBREAK_DECODED, step, shift, encoded_index, encoded_length, decoded_index, decoded_str_max_size);
        #endif  // VERBOSE_DEBUG

            encoded_index++;
        } else if (encoded_index + strlen(AMKD_LINEBREAK_ENCODED) - 1 <= encoded_length
                && !strncmp(encoded_str + encoded_index, AMKD_LINEBREAK_ENCODED, strlen(AMKD_LINEBREAK_ENCODED))) {
            // translate symbolic line break to literal one
            if (decoded_index + strlen(AMKD_LINEBREAK_DECODED) <= decoded_str_max_size) {
                strcpy(*decoded_str + decoded_index, AMKD_LINEBREAK_DECODED);
            } else {
                free(*decoded_str);
                return AMKD_ERROR_INSUFFICIENT_BUFFER;
            }
        #ifdef VERBOSE_DEBUG
            printf(" Decoding: '%s'->'%s' (step: %d, shift: %d). Position: in %d/%d, out %d/%d\n", 
                AMKD_LINEBREAK_ENCODED, AMKD_LINEBREAK_DECODED, step, shift, encoded_index, encoded_length,
                decoded_index, decoded_str_max_size);
        #endif  // VERBOSE_DEBUG

            decoded_index += strlen(AMKD_LINEBREAK_DECODED);
            encoded_index += strlen(AMKD_LINEBREAK_ENCODED);
        } else {
            if (decoded_index + 1 <= decoded_str_max_size) {
                // decipher a character
                (*decoded_str)[decoded_index] = encoded_str[encoded_index] + shift;
            } else {
                // handle errors
                free(*decoded_str);
                return AMKD_ERROR_INSUFFICIENT_BUFFER;
            }
            // detect suspicious characters
            if (warning_flags != NULL) {
                if ((*decoded_str)[decoded_index] < 32 || (*decoded_str)[decoded_index] == 127) {
                    *warning_flags |= AMKD_WARNING_CONTROL_CHARS;
                }
            }
        #ifdef VERBOSE_DEBUG
            printf(" Decoding: '%c'->'%c' (step: %d, shift: %d). Position: in %d/%d, out %d/%d\n", 
                encoded_str[encoded_index], (*decoded_str)[decoded_index], step, shift, encoded_index, encoded_length,
                decoded_index, decoded_str_max_size);
        #endif  // VERBOSE_DEBUG

            encoded_index++;
            decoded_index++;
            step++;
        }
    }

    if (decoded_index < decoded_str_max_size) {
        (*decoded_str)[decoded_index] = '\0';
    } else {
        (*decoded_str)[decoded_str_max_size] = '\0';
    }
#ifdef DEBUG
    printf(" Succesfully decoded\n");
#endif  // DEBUG
    return AMKD_ERROR_NONE;
}

AMkd_error_code AMkd_encode(const char *decoded_str, char **encoded_str, AMkd_config config, AMkd_warning_flags *warning_flags)
{
#ifdef DEBUG
    printf("Encoding...\n");
#endif  // DEBUG
    if (warning_flags != NULL) {
        *warning_flags = AMKD_WARNING_NONE;
    }
    if (decoded_str == NULL) {
        return AMKD_ERROR_MISSING_INPUT;
    }
    if (config == AMKD_CONFIG_NONE) {
        return AMKD_ERROR_MISSING_CONFIG;
    }
#ifdef DEBUG
    printf(" Config: %d\n", config);
#endif  // DEBUG
    unsigned decoded_length = strlen(decoded_str),
        decoded_index = 0,
        encoded_index = 0,
        step = 1,
        line_break_count = 0,
        encoded_str_max_size = AMkd_calculate_size(decoded_str, AMKD_CONFIG_NONE, config);
#ifdef DEBUG
    printf(" Max encoded_str size: %d\n", encoded_str_max_size);
#endif  // DEBUG
    *encoded_str = (char *)malloc((encoded_str_max_size + 1) * sizeof(char));
    if (*encoded_str == NULL) {
        return AMKD_ERROR_OUT_OF_MEMORY;
    }
    encoded_index = snprintf(*encoded_str, encoded_str_max_size, "{<C:%d>}\n", config);
    while (decoded_index < decoded_length) {
    #ifdef DEBUG
        //printf(" Encoded: %d/%d (result state: %d/%d)\n", decoded_index, decoded_length, encoded_index, encoded_str_max_size);
    #endif  // DEBUG
        int movement = (step + 1) / 2;
        if (step % 2 == 0) {
            movement *= -1;
        }
        // detect literal line break
        if (decoded_str[decoded_index] == '\n') {
            if (encoded_index + 3 <= encoded_str_max_size) {
                // translate literal line breaks to symbolic line breaks
                (*encoded_str)[encoded_index] = '<';
                (*encoded_str)[encoded_index + 1] = 'E';
                (*encoded_str)[encoded_index + 2] = '>';
            } else {
                free(*encoded_str);
                return AMKD_ERROR_INSUFFICIENT_BUFFER;
            }
            decoded_index++;
            encoded_index += 3;
            line_break_count++;
            // make literal line break every [step_count] symbolic line breaks
            if (line_break_count >= config) {
                if (encoded_index + 1 <= encoded_str_max_size) {
                    (*encoded_str)[encoded_index] = '\n';
                } else {
                    free(*encoded_str);
                    return AMKD_ERROR_INSUFFICIENT_BUFFER;
                }
                encoded_index += 1;
                line_break_count = 0;
            }
        } else {
            // detect suspicious characters
            if (decoded_str[decoded_index] < 32 || decoded_str[decoded_index] == 127) {
                if (warning_flags != NULL) {
                    *warning_flags |= AMKD_WARNING_CONTROL_CHARS;
                }
            }
            if (encoded_index + 1 <= encoded_str_max_size) {
                (*encoded_str)[encoded_index] = decoded_str[decoded_index] + movement;
            } else {
                free(*encoded_str);
                return AMKD_ERROR_INSUFFICIENT_BUFFER;
            }
            decoded_index++;
            encoded_index++;
            step++;
        }
        if (step > config) {
            step = 1;
        }
    }
    if (encoded_index + 1 <= encoded_str_max_size) {
        (*encoded_str)[encoded_index] = '\n';
        encoded_index += 1;
    } else {
        free(*encoded_str);
        return AMKD_ERROR_INSUFFICIENT_BUFFER;
    }
    if (encoded_index < encoded_str_max_size) {
        (*encoded_str)[encoded_index] = '\0';
    } else {
        (*encoded_str)[encoded_str_max_size] = '\0';
    }
#ifdef DEBUG
    printf(" Succesfully encoded\n");
#endif  // DEBUG
    return AMKD_ERROR_NONE;
}

void AMkd_deallocate_result(char *result_str)
{
    free(result_str);
}

AMkd_error_code AMkd_strip_header(char *encoded_str, AMkd_warning_flags *warning_flags)
{
    if (warning_flags != NULL) {
        *warning_flags = AMKD_WARNING_NONE;
    }
    if (encoded_str == NULL) {
        return AMKD_ERROR_MISSING_INPUT;
    }
    char trash_char;
    int trash_int;
    unsigned string_length, characters_read;
    string_length = strlen(encoded_str);
    if (sscanf(encoded_str, "{<%c:%d>}%n", &trash_char, &trash_int, &characters_read) != 2) {
        if (warning_flags != NULL) {
            *warning_flags |= AMKD_WARNING_MISSING_HEADER;
        }
        return AMKD_ERROR_NONE;
    }
    if (characters_read < string_length) {
        if ((encoded_str)[characters_read] == '\n') {
            characters_read++;
        }
    }
    memmove(encoded_str, encoded_str + characters_read, string_length - characters_read + 1);
    return AMKD_ERROR_NONE;
}

AMkd_error_code AMkd_detect_encoding(const char *encoded_str, AMkd_config *config, AMkd_warning_flags *warning_flags)
{
    if (warning_flags != NULL) {
        *warning_flags = AMKD_WARNING_NONE;
    }
    if (encoded_str == NULL) {
        return AMKD_ERROR_MISSING_INPUT;
    }
    char trash_char;
    if (sscanf(encoded_str, "{<%c:%d>}", &trash_char, config) == 2) {
        // case 1. header present
        return AMKD_ERROR_NONE;
    } else {
        // case 2. <E> (not) present
        /*
            [TODO]
        */
        // case 3. comparing offsets with "OBJECT=name/nname:TYPE"
        /*
            [TODO]
        */
        *config = AMKD_CONFIG_NONE;
        if (warning_flags != NULL) {
            *warning_flags |= AMKD_WARNING_UNKNOWN_ENCODING;
        }
        return AMKD_ERROR_NONE;
    }
}

const char *AMkd_get_error_string(AMkd_error_code code)
{
    switch (code) {
        case AMKD_ERROR_NONE: {
            return "No errors. [AMKD_ERROR_NONE]";
        }
        case AMKD_ERROR_MISSING_CONFIG: {
            return "Provided setting (cycle step count) of 0 for encoding or decoding. [AMKD_ERROR_MISSING_CONFIG]";
        }
        case AMKD_ERROR_OUT_OF_MEMORY: {
            return "No memory available for operation. [AMKD_ERROR_OUT_OF_MEMORY]";
        }
        case AMKD_ERROR_MISSING_INPUT: {
            return "Provided null pointer instead of input string. [AMKD_ERROR_MISSING_INPUT]";
        }
        case AMKD_ERROR_INSUFFICIENT_BUFFER: {
            return "Incorrect length of internal output buffer calculated by the library. [AMKD_ERROR_INSUFFICIENT_BUFFER]";
        }
        default: {
            return "Unknown error.";
        }
    }
}

static unsigned count_set_bits(unsigned number)
{
    // Brian Kernighan's algorithm
    unsigned set_bits_count = 0;
    while (number != 0) {
        number &= number - 1;
    }
    return set_bits_count;
}

const char *AMkd_get_warning_string(AMkd_warning_flags flags)
{
    if (count_set_bits((unsigned)flags) > 1) {
        unsigned least_significant_bit_position = 0;
        while ((flags & 0x1) == 0) {
            flags >>= 1;
            least_significant_bit_position++;
        }
        flags = 1 << least_significant_bit_position;
    }
    switch (flags) {
        case AMKD_WARNING_NONE: {
            return "No warnings. [AMKD_WARNING_NONE]";
        }
        case AMKD_WARNING_SURPLUS_CONFIG: {
            return "Decoding setting present both in input string header and function argument. [AMKD_WARNING_SURPLUS_CONFIG]";
        }
        case AMKD_WARNING_MISSING_HEADER: {
            return "No header to strip. [AMKD_WARNING_MISSING_HEADER]";
        }
        case AMKD_WARNING_CONTROL_CHARS: {
            return "Control characters present in decoded string. [AMKD_WARNING_CONTROL_CHARS]";
        }
        case AMKD_WARNING_UNKNOWN_ENCODING: {
            return "Encoding setting of input string impossible to detect programatically. [AMKD_WARNING_UNKNOWN_ENCODING]";
        }
        default: {
            return "Unknown warning.";
        }
    }
}
