#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../include/AMkd.h"

const AMkd_config AMKD_CONFIG_DEFAULT = {6, 'C'};
const AMkd_config AMKD_CONFIG_NONE = {0, 'C'};

typedef enum AMkd_config_compare_mode {
    AMKD_COMPARE_WHOLE,
    AMKD_COMPARE_STEPS,
    AMKD_COMPARE_LETTER
} AMkd_config_compare_mode;

static bool AMkd_compare_configs(AMkd_config left, AMkd_config right, AMkd_config_compare_mode compare_mode)
{
    switch (compare_mode) {
        case AMKD_COMPARE_LETTER: {
            return left.letter == right.letter;
        }
        case AMKD_COMPARE_STEPS: {
            return left.step_count == right.step_count;
        }
        default: {
            return left.letter == right.letter && left.step_count == right.step_count;
        }
    }
}

static unsigned AMkd_calculate_size(const char *input_str, AMkd_config input_settings, AMkd_config output_settings)
{
#ifdef DEBUG
    printf(" Calculating size...\n");
#endif // DEBUG
    unsigned string_length = strlen(input_str) + 1;
    if (input_settings.step_count == output_settings.step_count) {
        return string_length;
    } else if (output_settings.step_count == 0) {
        const char *sought_sequence = "<E>";
        const char *replace_sequence = "\r\n";
        int line_break_count = 0;
        const char *line_break = strstr(input_str, sought_sequence);
        while (line_break != NULL) {
            line_break_count++;
            line_break = strstr(line_break + 1, sought_sequence);
        }
    #ifdef DEBUG
        printf(" variables:\n");
        printf("  string_length: %d\n", string_length);
        printf("  line_break_count: %d\n", line_break_count);
        printf("  strlen(replace_sequence): %d\n", strlen(replace_sequence));
        printf("  strlen(sought_sequence): %d\n", strlen(sought_sequence));
    #endif // DEBUG
        return (string_length + line_break_count * (strlen(replace_sequence) - strlen(sought_sequence)));
    } else {
        const char *sought_sequence = "\r\n";
        const char *replace_sequence = "<E>";
        int line_break_count = 0;
        const char *line_break = strstr(input_str, sought_sequence);
        while (line_break != NULL) {
            line_break_count++;
            line_break = strstr(line_break + 1, sought_sequence);
        }
        int header_length = strlen("{<C:>}\r\n");
        int step_count_length = 1;
        if (output_settings.step_count < 0) {
            step_count_length++;
        }
        for (int step_count = output_settings.step_count; step_count > 9; step_count /= 10) {
            step_count_length++;
        }
        header_length += step_count_length;
    #ifdef DEBUG
        printf(" variables:\n");
        printf("  string_length: %d\n", string_length);
        printf("  line_break_count: %d\n", line_break_count);
        printf("  strlen(replace_sequence): %d\n", strlen(replace_sequence));
        printf("  strlen(sought_sequence): %d\n", strlen(sought_sequence));
        printf("  output_settings.step_count: %d\n", output_settings.step_count);
        printf("  header_length: %d\n", header_length);
    #endif // DEBUG
        return (string_length + line_break_count * (strlen(replace_sequence) - strlen(sought_sequence))
            + (line_break_count / output_settings.step_count + 1) * 2 + header_length);
    }
}

AMkd_error AMkd_decode(const char *encoded_str, char **decoded_str, AMkd_config settings)
{
#ifdef DEBUG
    printf("Decoding...\n");
#endif // DEBUG
    AMkd_error local_errno = AMKD_ERROR_NONE;
    if (AMkd_compare_configs(settings, AMKD_CONFIG_NONE, AMKD_COMPARE_STEPS)) {
        AMkd_config header_settings = AMKD_CONFIG_NONE;
        AMkd_detect_encoding(encoded_str, &header_settings);
        if (AMkd_compare_configs(settings, AMKD_CONFIG_NONE, AMKD_COMPARE_STEPS)) {
            return AMKD_ERROR_MISSING_SETTINGS;
        }
        settings = header_settings;
    } else {
        AMkd_config header_settings = AMKD_CONFIG_NONE;
        AMkd_detect_encoding(encoded_str, &header_settings);
        if (!AMkd_compare_configs(settings, AMKD_CONFIG_NONE, AMKD_COMPARE_STEPS)) {
            local_errno = AMKD_WARNING_SURPLUS_SETTINGS;
        }
    }
#ifdef DEBUG
    printf(" Settings: %c:%d\n", settings.letter, settings.step_count);
#endif // DEBUG
    unsigned encoded_length = strlen(encoded_str),
        encoded_index = 0,
        decoded_index = 0,
        step = 1,
        decoded_str_max_size = AMkd_calculate_size(encoded_str, settings, AMKD_CONFIG_NONE);
#ifdef DEBUG
    printf(" Max decoded_str size: %d\n", decoded_str_max_size);
#endif // DEBUG
    *decoded_str = (char *)malloc((decoded_str_max_size + 1) * sizeof(char));
    if (*decoded_str == NULL) {
        return AMKD_ERROR_OUT_OF_MEMORY;
    }
    {
        char trash_char;
        int trash_int;
        unsigned header_length = 0;
        if (sscanf(encoded_str, "{<%c:%d>}%n", &trash_char, &trash_int, &header_length) == 2) {
            encoded_index += header_length;
        }
    }
    while (encoded_index < encoded_length && decoded_index < decoded_str_max_size) {
    #ifdef DEBUG
        //printf(" Decoded: %d/%d (result state: %d/%d)\n", encoded_index, encoded_length, decoded_index, decoded_str_max_size);
        //printf(" Step: %d, current encoded character: %c\n", step, encoded_str[encoded_index]);
    #endif // DEBUG
        int movement = (step + 1) / 2;
        if (step % 2 == 1) {
            movement *= -1;
        }
        if (encoded_str[encoded_index] == '\r' || encoded_str[encoded_index] == '\n') {
            encoded_index++;
        } else {
            bool detected_line_break = false;
            //detect symbolic line break
            if (encoded_index + 2 <= encoded_length) {
                if (encoded_str[encoded_index] == '<' && encoded_str[encoded_index + 1] == 'E' && encoded_str[encoded_index + 2] == '>') {
                    detected_line_break = true;
                }
            }
            if (detected_line_break) {
                //translate symbolic line breaks to CRLF
                if (decoded_index + 2 <= decoded_str_max_size) {
                    (*decoded_str)[decoded_index] = '\r';
                    (*decoded_str)[decoded_index + 1] = '\n';
                    decoded_index += 2;
                    encoded_index += 3;
                }
            } else {
                (*decoded_str)[decoded_index] = encoded_str[encoded_index] + movement;
                if ((*decoded_str)[decoded_index] < 32) {
                    local_errno = AMKD_WARNING_CONTROL_CHARS;
                }
                encoded_index++;
                decoded_index++;
                step++;
            }
            if (step > settings.step_count) {
                step = 1;
            }
        }
    }
    if (decoded_index < decoded_str_max_size) {
        (*decoded_str)[decoded_index] = '\0';
    } else {
        (*decoded_str)[decoded_str_max_size] = '\0';
    }
#ifdef DEBUG
    printf(" Succesfully decoded\n");
#endif // DEBUG
    return local_errno;
}

AMkd_error AMkd_encode(const char *decoded_str, char **encoded_str, AMkd_config settings)
{
#ifdef DEBUG
    printf("Encoding...\n");
#endif // DEBUG
    if (AMkd_compare_configs(settings, AMKD_CONFIG_NONE, AMKD_COMPARE_STEPS)) {
        return AMKD_ERROR_MISSING_SETTINGS;
    }
#ifdef DEBUG
    printf(" Settings: %c:%d\n", settings.letter, settings.step_count);
#endif // DEBUG
    unsigned decoded_length = strlen(decoded_str),
        decoded_index = 0,
        encoded_index = 0,
        step = 1,
        line_break_count = 0,
        encoded_str_max_size = AMkd_calculate_size(decoded_str, AMKD_CONFIG_NONE, settings);
#ifdef DEBUG
    printf(" Max encoded_str size: %d\n", encoded_str_max_size);
#endif // DEBUG
    *encoded_str = (char *)malloc((encoded_str_max_size + 1) * sizeof(char));
    if (*encoded_str == NULL) {
        return AMKD_ERROR_OUT_OF_MEMORY;
    }
    encoded_index = snprintf(*encoded_str, encoded_str_max_size, "{<%c:%d>}\r\n", settings.letter, settings.step_count);
    while (decoded_index < decoded_length && encoded_index < encoded_str_max_size) {
    #ifdef DEBUG
        //printf(" Encoded: %d/%d (result state: %d/%d)\n", decoded_index, decoded_length, encoded_index, encoded_str_max_size);
    #endif // DEBUG
        int movement = (step + 1) / 2;
        if (step % 2 == 0) {
            movement *= -1;
        }
        bool detected_line_break = false;
        //detect CRLF line break
        if (decoded_index + 1 <= decoded_length) {
            if (decoded_str[decoded_index] == '\r' && decoded_str[decoded_index + 1] == '\n') {
                detected_line_break = true;
            }
        }
        if (detected_line_break) {
            //translate literal line breaks to symbolic line breaks
            if (encoded_index + 2 < encoded_str_max_size) {
                (*encoded_str)[encoded_index] = '<';
                (*encoded_str)[encoded_index + 1] = 'E';
                (*encoded_str)[encoded_index + 2] = '>';
            }
            decoded_index += 2;
            encoded_index += 3;
            line_break_count++;
            //makes literal line break every [step_count] coded line breaks
            if (line_break_count >= settings.step_count) {
                if (encoded_index + 2 < encoded_str_max_size) {
                    (*encoded_str)[encoded_index] = '\r';
                    (*encoded_str)[encoded_index + 1] = '\n';
                }
                encoded_index += 2;
                line_break_count = 0;
            }
        } else {
            (*encoded_str)[encoded_index] = decoded_str[decoded_index] + movement;
            decoded_index++;
            encoded_index++;
            step++;
        }
        if (step > settings.step_count) {
            step = 1;
        }
    }
    if (encoded_index + 1 <= encoded_str_max_size) {
        (*encoded_str)[encoded_index] = '\r';
        (*encoded_str)[encoded_index + 1] = '\n';
        encoded_index += 2;
    }
    if (encoded_index < encoded_str_max_size) {
        (*encoded_str)[encoded_index] = '\0';
    } else {
        (*encoded_str)[encoded_str_max_size] = '\0';
    }
#ifdef DEBUG
    printf(" Succesfully encoded\n");
#endif // DEBUG
    return AMKD_ERROR_NONE;
}

void AMkd_deallocate_result(char *result_str)
{
    free(result_str);
}

AMkd_error AMkd_strip_header(char *encoded_str)
{
    char trash_char;
    int trash_int;
    unsigned string_length, characters_read;
    string_length = strlen(encoded_str);
    if (sscanf(encoded_str, "{<%c:%d>}%n", &trash_char, &trash_int, &characters_read) != 2) {
        return AMKD_WARNING_MISSING_HEADER;
    }
    if (characters_read < string_length) {
        if ((encoded_str)[characters_read] == '\n') {
            characters_read++;
        }
    }
    memmove(encoded_str, encoded_str + characters_read, string_length - characters_read + 1);
    return AMKD_ERROR_NONE;
}

AMkd_error AMkd_detect_encoding(const char *encoded_str, AMkd_config *settings)
{
    if (sscanf(encoded_str, "{<%c:%d>}", &(settings->letter), &(settings->step_count)) == 2) {
        //case 1. header present
        return AMKD_ERROR_NONE;
    } else {
        //case 2. <E> (not) present
        /*
            [TODO]
        */
        //case 3. comparing offsets with "OBJECT=name/nname:TYPE"
        /*
            [TODO]
        */
        settings->letter = AMKD_CONFIG_NONE.letter;
        settings->step_count = AMKD_CONFIG_NONE.step_count;
        return AMKD_WARNING_UNKNOWN_ENCODING;
    }
}
