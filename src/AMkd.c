#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../include/AMkd.h"

const AMkd_cycle_param AMKD_CONFIG_DEFAULT = 6;
const AMkd_cycle_param AMKD_CONFIG_NONE = 0;

enum AMkd_line_ending {
    AMKD_LINE_ENDING_UNKNOWN = 0,
    AMKD_LINE_ENDING_CRLF,
    AMKD_LINE_ENDING_LF,
    AMKD_LINE_ENDING_CR
};
typedef enum AMkd_line_ending AMkd_line_ending;

static AMkd_line_ending AMkd_detect_line_ending(const char *input_str)
{
    char *lf_position = NULL, *cr_position = NULL;
    lf_position = strchr(input_str, '\n');
    if (lf_position != NULL) {
        if (lf_position != input_str) {
            if (input_str[lf_position - input_str - 1] == '\r') {
                return AMKD_LINE_ENDING_CRLF;
            } else {
                return AMKD_LINE_ENDING_LF;
            }
        } else {
            return AMKD_LINE_ENDING_LF;
        }
    } else {
        cr_position = strchr(input_str, '\r');
        if (cr_position != NULL) {
            return AMKD_LINE_ENDING_CR;
        } else {
            return AMKD_LINE_ENDING_UNKNOWN;
        }
    }
}

static unsigned AMkd_calculate_size(const char *input_str, AMkd_cycle_param input_setting, AMkd_cycle_param output_setting)
{
#ifdef DEBUG
    printf(" Calculating size...\n");
#endif // DEBUG
    unsigned string_length = strlen(input_str) + 1;
    if (input_setting == output_setting) {
        return string_length;
    } else if (output_setting == AMKD_CONFIG_NONE) {
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
        char *sought_sequence = NULL;
        const char *replace_sequence = "<E>";
        int line_break_count = 0;
        switch (AMkd_detect_line_ending(input_str)) {
            case AMKD_LINE_ENDING_LF: {
                sought_sequence = "\n";
                break;
            }
            case AMKD_LINE_ENDING_CRLF: {
                sought_sequence = "\r\n";
                break;
            }
            case AMKD_LINE_ENDING_CR: {
                sought_sequence = "\r";
                break;
            }
            case AMKD_LINE_ENDING_UNKNOWN: {
                sought_sequence = "";
                break;
            }
        }
        if (strlen(sought_sequence) > 0) {
            const char *line_break = strstr(input_str, sought_sequence);
            while (line_break != NULL) {
                line_break_count++;
                line_break = strstr(line_break + 1, sought_sequence);
            }
        }
        int header_length = strlen("{<C:>}\r\n");
        int step_count_length = 1;
        for (int step_count = output_setting; step_count > 9; step_count /= 10) {
            step_count_length++;
        }
        header_length += step_count_length;
    #ifdef DEBUG
        printf(" variables:\n");
        printf("  string_length: %d\n", string_length);
        printf("  line_break_count: %d\n", line_break_count);
        printf("  strlen(replace_sequence): %d\n", strlen(replace_sequence));
        printf("  strlen(sought_sequence): %d\n", strlen(sought_sequence));
        printf("  output_setting: %d\n", output_setting);
        printf("  header_length: %d\n", header_length);
    #endif // DEBUG
        return (string_length + line_break_count * (strlen(replace_sequence) - strlen(sought_sequence))
            + (line_break_count / output_setting + 1) * 2 + header_length);
    }
}

AMkd_error_code AMkd_decode(const char *encoded_str, char **decoded_str, AMkd_cycle_param setting, AMkd_warning_flags *warning_flags)
{
#ifdef DEBUG
    printf("Decoding...\n");
#endif // DEBUG
    if (warning_flags != NULL) {
        *warning_flags = AMKD_WARNING_NONE;
    }
    if (encoded_str == NULL) {
        return AMKD_ERROR_MISSING_INPUT;
    }
    if (setting == AMKD_CONFIG_NONE) {
        //zero-step setting argument: read setting from encoded string header
        AMkd_cycle_param header_setting = AMKD_CONFIG_NONE;
        AMkd_error_code inner_error;
        if ((inner_error = AMkd_detect_encoding(encoded_str, &header_setting, warning_flags)) != AMKD_ERROR_NONE) {
            return inner_error;
        }
        if (setting == AMKD_CONFIG_NONE) {
            return AMKD_ERROR_MISSING_SETTING;
        }
        setting = header_setting;
    } else {
        AMkd_cycle_param header_setting = AMKD_CONFIG_NONE;
        AMkd_error_code inner_error;
        if ((inner_error = AMkd_detect_encoding(encoded_str, &header_setting, warning_flags)) != AMKD_ERROR_NONE) {
            return inner_error;
        }
        if (setting != AMKD_CONFIG_NONE) {
            if (warning_flags != NULL) {
                *warning_flags |= AMKD_WARNING_SURPLUS_SETTING;
            }
        }
    }
#ifdef DEBUG
    printf(" Setting: %d\n", setting);
#endif // DEBUG
    unsigned encoded_length = strlen(encoded_str),
        encoded_index = 0,
        decoded_index = 0,
        step = 1,
        decoded_str_max_size = AMkd_calculate_size(encoded_str, setting, AMKD_CONFIG_NONE);
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
    bool encountered_standard_line_ending = false;
    while (encoded_index < encoded_length) {
    #ifdef DEBUG
        //printf(" Decoded: %d/%d (result state: %d/%d)\n", encoded_index, encoded_length, decoded_index, decoded_str_max_size);
        //printf(" Step: %d, current encoded character: %c\n", step, encoded_str[encoded_index]);
    #endif // DEBUG
        int movement = (step + 1) / 2;
        if (step % 2 == 1) {
            movement *= -1;
        }
        if (encoded_str[encoded_index] == '\r' || encoded_str[encoded_index] == '\n') {
            if (!encountered_standard_line_ending) {
                encountered_standard_line_ending = true;
            }
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
                //translate symbolic line breaks to literal CRLF
                if (decoded_index + 2 <= decoded_str_max_size) {
                    (*decoded_str)[decoded_index] = '\r';
                    (*decoded_str)[decoded_index + 1] = '\n';
                } else {
                    free(*decoded_str);
                    return AMKD_ERROR_INSUFFICIENT_BUFFER;
                }
                decoded_index += 2;
                encoded_index += 3;
            } else {
                if (decoded_index + 1 <= decoded_str_max_size) {
                    //write translated character
                    (*decoded_str)[decoded_index] = encoded_str[encoded_index] + movement;
                } else {
                    //handle errors
                    free(*decoded_str);
                    return AMKD_ERROR_INSUFFICIENT_BUFFER;
                }
                //detect suspicious characters
                if ((*decoded_str)[decoded_index] < 32 || (*decoded_str)[decoded_index] == 127) {
                    if (warning_flags != NULL) {
                        *warning_flags |= AMKD_WARNING_CONTROL_CHARS;
                    }
                }
                encoded_index++;
                decoded_index++;
                step++;
            }
            if (step > setting) {
                step = 1;
            }
        }
    }
    if (!encountered_standard_line_ending) {
        if (warning_flags != NULL) {
            *warning_flags |= AMKD_WARNING_UNKNOWN_LINE_ENDING;
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
    return AMKD_ERROR_NONE;
}

AMkd_error_code AMkd_encode(const char *decoded_str, char **encoded_str, AMkd_cycle_param setting, AMkd_warning_flags *warning_flags)
{
#ifdef DEBUG
    printf("Encoding...\n");
#endif // DEBUG
    if (warning_flags != NULL) {
        *warning_flags = AMKD_WARNING_NONE;
    }
    if (decoded_str == NULL) {
        return AMKD_ERROR_MISSING_INPUT;
    }
    if (setting == AMKD_CONFIG_NONE) {
        return AMKD_ERROR_MISSING_SETTING;
    }
#ifdef DEBUG
    printf(" Setting: %d\n", setting);
#endif // DEBUG
    unsigned decoded_length = strlen(decoded_str),
        decoded_index = 0,
        encoded_index = 0,
        step = 1,
        line_break_count = 0,
        encoded_str_max_size = AMkd_calculate_size(decoded_str, AMKD_CONFIG_NONE, setting);
#ifdef DEBUG
    printf(" Max encoded_str size: %d\n", encoded_str_max_size);
#endif // DEBUG
    *encoded_str = (char *)malloc((encoded_str_max_size + 1) * sizeof(char));
    if (*encoded_str == NULL) {
        return AMKD_ERROR_OUT_OF_MEMORY;
    }
    encoded_index = snprintf(*encoded_str, encoded_str_max_size, "{<C:%d>}\r\n", setting);
    AMkd_line_ending line_ending = AMkd_detect_line_ending(decoded_str);
    if (line_ending == AMKD_LINE_ENDING_UNKNOWN) {
        if (warning_flags != NULL) {
            *warning_flags |= AMKD_WARNING_UNKNOWN_LINE_ENDING;
        }
    }
    while (decoded_index < decoded_length) {
    #ifdef DEBUG
        //printf(" Encoded: %d/%d (result state: %d/%d)\n", decoded_index, decoded_length, encoded_index, encoded_str_max_size);
    #endif // DEBUG
        int movement = (step + 1) / 2;
        if (step % 2 == 0) {
            movement *= -1;
        }
        bool detected_line_break = false;
        //detect literal line break
        switch (line_ending) {
            case AMKD_LINE_ENDING_LF: {
                if (decoded_str[decoded_index] == '\n') {
                    detected_line_break = true;
                }
                break;
            }
            case AMKD_LINE_ENDING_CRLF: {
                if (decoded_index + 1 <= decoded_length) {
                    if (decoded_str[decoded_index] == '\r' && decoded_str[decoded_index + 1] == '\n') {
                        detected_line_break = true;
                    }
                }
                break;
            }
            case AMKD_LINE_ENDING_CR: {
                if (decoded_str[decoded_index] == '\r') {
                    detected_line_break = true;
                }
                break;
            }
        }
        if (detected_line_break) {
            if (encoded_index + 3 <= encoded_str_max_size) {
                //translate literal line breaks to symbolic line breaks
                (*encoded_str)[encoded_index] = '<';
                (*encoded_str)[encoded_index + 1] = 'E';
                (*encoded_str)[encoded_index + 2] = '>';
            } else {
                free(*encoded_str);
                return AMKD_ERROR_INSUFFICIENT_BUFFER;
            }
            if (line_ending == AMKD_LINE_ENDING_CRLF) {
                decoded_index += 2;
            } else {
                decoded_index++;
            }
            encoded_index += 3;
            line_break_count++;
            //make literal line break every [step_count] symbolic line breaks
            if (line_break_count >= setting) {
                if (encoded_index + 2 <= encoded_str_max_size) {
                    (*encoded_str)[encoded_index] = '\r';
                    (*encoded_str)[encoded_index + 1] = '\n';
                } else {
                    free(*encoded_str);
                    return AMKD_ERROR_INSUFFICIENT_BUFFER;
                }
                encoded_index += 2;
                line_break_count = 0;
            }
        } else {
            //detect suspicious characters
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
        if (step > setting) {
            step = 1;
        }
    }
    if (encoded_index + 2 <= encoded_str_max_size) {
        (*encoded_str)[encoded_index] = '\r';
        (*encoded_str)[encoded_index + 1] = '\n';
        encoded_index += 2;
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
#endif // DEBUG
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

AMkd_error_code AMkd_detect_encoding(const char *encoded_str, AMkd_cycle_param *setting, AMkd_warning_flags *warning_flags)
{
    if (warning_flags != NULL) {
        *warning_flags = AMKD_WARNING_NONE;
    }
    if (encoded_str == NULL) {
        return AMKD_ERROR_MISSING_INPUT;
    }
    char trash_char;
    if (sscanf(encoded_str, "{<%c:%d>}", &trash_char, setting) == 2) {
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
        setting = AMKD_CONFIG_NONE;
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
        case AMKD_ERROR_MISSING_SETTING: {
            return "Provided setting with cycle step count of 0 for encoding or decoding. [AMKD_ERROR_MISSING_SETTING]";
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
    //Brian Kernighan's algorithm
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
        case AMKD_WARNING_SURPLUS_SETTING: {
            return "Decoding setting present both in input string header and function argument. [AMKD_WARNING_SURPLUS_SETTING]";
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
        case AMKD_WARNING_UNKNOWN_LINE_ENDING: {
            return "Unknown line ending sequence in input string (or no line endings at all). [AMKD_WARNING_UNKNOWN_LINE_ENDING]";
        }
        default: {
            return "Unknown warning.";
        }
    }
}
