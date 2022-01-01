#ifndef AMKD_H
#define AMKD_H

/*! \file AMkd.h
    \brief AMkd library header file.

    Provides documented function declarations and description of used structures, global variables
    and enumerations for library users.
 */


/*! \brief Return code for signaling successful execution or failure of library functions.

    Only describing critical errors, i.e. these interrupting function execution. \n
    Zero indicates no error and a positive value means that error has occured.
 */
enum AMkd_error_code {
    AMKD_ERROR_NONE = 0,  //!< No error.
    AMKD_ERROR_MISSING_CONFIG,  //!< Error: no cycle param provided for encoding/decoding.
    AMKD_ERROR_OUT_OF_MEMORY,  //!< Error: no memory available for allocation for output string.
    AMKD_ERROR_MISSING_INPUT,  //!< Error: received NULL as encoded/decoded string.
    AMKD_ERROR_INSUFFICIENT_BUFFER,  //!< Error: output buffer allocated by de-/encoding function turned out to be too small.
    AMKD_ERROR_MISSING_HEADER,  //!< Error: no header in provided string.
    AMKD_ERROR_ILLEGAL_HEADER,  //!< Error: found illegal config (0) in encoded string header. \sa AMkd_config
    AMKD_ERROR_AMBIGUOUS_OUTPUT,  //!< Error: encoded non-line break characters as a literal or symbolic line break. \sa AMkd_encode
};
/*! \brief Shorthand for #AMkd_error_code enumeration.
 */
typedef enum AMkd_error_code AMkd_error_code;

/*! \brief Binary warning flags set to be passed as an argument to library functions supporting warning handling.

    Describing non-critical errors, signaling suspicious, but correct data or redundancy of operations, etc. \n
    Zero indicating no warnings and consecutive bits (starting from the least significant) meaning different warnings. \n
    According to C language specification, maximum safe number of such flags is sixteen (not counting 0),
    as enum uses int type and int is guaranteed to be at least 16 bits long. \n
    Flags are to be set by OR-ing previous state of variable with new warning
    and to be checked by AND-ing the variable with a chosen flag.
 */
enum AMkd_warning_flags {
    AMKD_WARNING_NONE = 0,  //!< No warnings.
    AMKD_WARNING_CONTROL_CHARS = 1 << 0,  //!< Warning: control characters present in decoded string (may suggest incorrect decoding).
    AMKD_WARNING_UNKNOWN_ENCODING = 1 << 1,  //!< Warning: encoding impossible to detect. \sa AMkd_detect_encoding
};
/*! \brief Shorthand for #AMkd_warning_flags enumeration.
 */
typedef enum AMkd_warning_flags AMkd_warning_flags;

/*! \brief Structure holding encoding/decoding progress data.

    Used in #AMkd_decode and #AMkd_encode for providing additional information in the case of error.
 */
struct AMkd_progress {
    unsigned input_index;  //!< Position of last processed character from input buffer.
    unsigned output_index;  //!< Position of last character written to output buffer.
};
/*! \brief Shorthand for #AMkd_progress struct.
 */
typedef struct AMkd_progress AMkd_progress;

/*! \brief Encoding/decoding setting.

    Count of encoding steps in every cycle. Essential for correct operation. \n
    Value of 0 is logically illegal and thus used for expressing an empty setting.
 */
typedef unsigned int AMkd_config;

/*! \brief Default encoding/decoding setting.

    Should work for any script taken from AM's games.
*/
extern const AMkd_config AMKD_CONFIG_DEFAULT;

/*! \brief Empty encoding setting.

    Indicates no decoding and cannot be used as standalone decoding/encoding setting.
*/
extern const AMkd_config AMKD_CONFIG_NONE;


/*! \brief Decodes \a encoded_str and places it at \a *decoded_str.

    \param encoded_str Encoded null-terminated string.
    \param decoded_str Address of decoded null-terminated string "returned" by function.
    \param config Decoding setting. Cannot be empty (#AMKD_CONFIG_NONE). Any header in \a encoded_str is ignored. For defaults see #AMKD_CONFIG_DEFAULT
    \param error_details Pointer to structure used for storing decoding progress in the moment of error ocurrence. May be NULL (for ignoring information).
    \param warning_flags Pointer to value used for storing warning data. May be NULL (for ignoring warnings).

    \return Successful execution indicator or appriopriate error code.

    \warning Allocates memory for the decoded string: \a *decoded_str has to be manually freed after successful function execution.
    On failure, function does the clean-up by itself before returning. \sa AMkd_deallocate_result
 */
AMkd_error_code AMkd_decode(const char *encoded_str, char **decoded_str, AMkd_config config, AMkd_progress *error_details, AMkd_warning_flags *warning_flags);

/*! \brief Encodes \a decoded_str and places it at \a *encoded_str.

    \param decoded_str Decoded null-terminated string.
    \param encoded_str Address of encoded null-terminated string "returned" by function.
    \param config Encoding setting. Cannot be empty (#AMKD_CONFIG_NONE). For defaults see #AMKD_CONFIG_DEFAULT
    \param error_details Pointer to structure used for storing encoding progress in the moment of error ocurrence. May be NULL (for ignoring information).
    \param warning_flags Pointer to value used for storing warning data. May be NULL (for ignoring warnings).

    \return Successful execution indicator or appriopriate error code.

    \warning Allocates memory for the encoded string: \a *encoded_str has to be manually freed after successful function execution.
    On failure, function does the clean-up by itself before returning. \sa AMkd_deallocate_result
    One exception to this rule is #AMKD_ERROR_AMBIGUOUS_OUTPUT error. In such case \a *encoded_str contains partial output (null-terminated).
 */
AMkd_error_code AMkd_encode(const char *decoded_str, char **encoded_str, AMkd_config config, AMkd_progress *error_details, AMkd_warning_flags *warning_flags);

/*! \brief Releases memory allocated by #AMkd_decode or #AMkd_encode.

    Provided to obviate the ambiguity concerning deallocation method.

    \param result_str Decoded or encoded string "returned" by #AMkd_decode or #AMkd_encode.
 */
void AMkd_deallocate_result(char *result_str);

/*! \brief Parses header located at the beginning of \a *encoded_str.

    \param encoded_str Address of encoded null-terminated string containing a header.
    \param config Pointer to a variable where retrieved config value is to be stored. May be NULL (for ignoring the output).
    \param length Pointer to a variable where total header length is to be stored. May be NULL (for ignoring the output).

    \return Successful execution indicator or appriopriate error code.
 */
AMkd_error_code AMkd_parse_header(const char *encoded_str, AMkd_config *config, unsigned *length);

/*! \brief Tries to detect encoding in \a encoded_str and writes it into \a *config.

    For no or unknown encoding, the value pointed by \a config becomes equal to #AMKD_CONFIG_NONE. \n
    Also, unknown encoding is indicated by #AMKD_WARNING_UNKNOWN_ENCODING return code.

    \param encoded_str Encoded null-terminated string.
    \param config Address of config value. Must be provided by user.
    \param warning_flags Pointer to value used for storing warning data. May be NULL (for ignoring warnings).

    \return Successful execution indicator or appriopriate error code.

    \todo Add more checks for better detection.
 */
AMkd_error_code AMkd_detect_encoding(const char *encoded_str, AMkd_config *config, AMkd_warning_flags *warning_flags);

/*! \brief Translates given AMkd_error_code into human readable string.

    \param code Error code, for which the translation is requested.

    \return Error string associated with the given code.
 */
const char *AMkd_get_error_string(AMkd_error_code code);

/*! \brief Translates the first flag from AMkd_warning_flags (counting from the least significant bit) into human readable string.

    \param flags Binary flag register containing warning data. There should be only one flag set as the function returns only one string.

    \return Warning string associated with the least significant flag.
 */
const char *AMkd_get_warning_string(AMkd_warning_flags flags);

#endif  // AMKD_H
