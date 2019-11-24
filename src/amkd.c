#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../include/AMkd.h"

static bool AMkd_compare_configs(AMkd_config left, AMkd_config right)
{
    if (left.step_count == right.step_count && left.letter == right.letter) {
        return true;
    } else {
        return false;
    }
}

static unsigned AMkd_calculate_size(const char *input_str, AMkd_config input_settings, AMkd_config output_settings)
{
    unsigned string_length = strlen(input_str) + 1;
    if (input_settings.step_count != 0 && input_settings.step_count != 0) {
        return string_length;
    } else if (input_settings.step_count == 0) {
        ;
    }
}

AMkd_error AMkd_decode(const char *encoded_str, char **decoded_str, AMkd_config settings)
{
    AMkd_error local_errno = AMKD_ERROR_NONE;
    {
        int scanned_steps;
        char scanned_letter;
        if (sscanf(encoded_str, "{<%c:%d>}", &scanned_steps, &scanned_letter) == 2) {
            if (!AMkd_compare_configs(settings, AMKD_CONFIG_NONE)) {
                local_errno = AMKD_WARNING_SURPLUS_SETTINGS;
            }
            settings.step_count = scanned_steps;
            settings.letter = scanned_letter;
        }
    }
    for (int i = 1; i <= settings.step_count; i++) {
        int movement = (i + 1) / 2;
        if (i % 2 == 1) {
            movement *= -1;
        }

    }
    return local_errno;
}

AMkd_error AMkd_encode(const char *decoded_str, char **encoded_str, AMkd_config settings)
{
    ;
    return AMKD_ERROR_NONE;
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
    ;
    return AMKD_ERROR_NONE;
}
