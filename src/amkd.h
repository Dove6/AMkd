/*
 * Functions for:
 *  + decoding:
 *   - from file:
 *    - to string,
 *    - to file,
 *   + from string: [optional parameters]
 *    + to string,
 *    - to file,
 *  + encoding:
 *   - from file:
 *    - to string, [including header]
 *    - to file,
 *   + from string:
 *    + to string, [inluding header]
 *    - to file
 *  + stripping header
 *  + detecting encoding type
 */

/*
 * Possible errors:
 *  - insufficient memory, [critical]
 *  - no decoding parameters provided, [critical]
 *  - redundent deconding parameters provided (bonus: inconsistent), [warning-like]
 *  - characters out of boundaries (decoded characters below 32 ASCII code), [warning-like]
 *  - no header to strip, [warning-like]
 *  - no error
 */

/*
 * Configuration parameters:
 *  - number of steps/states,
 *  - reserved char (unknown purpose)
 */

//
typedef enum AMkd_error {
    AMKD_NO_ERROR = 0,
    AMKD_MISSING_PARAMETERS,
    AMKD_SURPLUS_PARAMETERS,
    AMKD_MISSING_HEADER,
    AMKD_OUT_OF_MEMORY,
    AMKD_CONTROL_CHARS
} AMkd_error;

//
typedef struct AMkd_config {
    int step_count;
    char letter;
} AMkd_config;

//
int AMkd_decode(const char *encoded_str, char *decoded_str, AMkd_config *decoding_param);

//
int AMkd_encode(const char *decoded_str, char *encoded_str, AMkd_config *encoding_param);

//
int AMkd_strip_header(char *encoded_str);

//
int AMkd_detect_encoding(const char *encoded_str);
