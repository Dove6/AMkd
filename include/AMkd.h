/*! \file AMkd.h
    \brief AMkd library header file.

    Provides documented function declarations and description of used structures, global variables
    and enumerations for library users.
 */


/*! \brief Error codes set in #AMkd_error_info structures.

    Only describing critical errors, i.e. these interrupting function execution. \n
    Zero indicating no error and positive values when error is present.
 */
enum AMkd_error_code {
    AMKD_ERROR_NONE = 0, //!< No error.
    AMKD_ERROR_MISSING_SETTINGS, //!< Error: no settings provided for encoding/decoding.
    AMKD_ERROR_OUT_OF_MEMORY, //!< Error: no memory available for allocation for output string.
    AMKD_ERROR_MISSING_INPUT, //!< Error: received NULL as encoded/decoded string.
    AMKD_ERROR_INSUFFICIENT_BUFFER //!< Error: output buffer allocated by de-/encoding turned out to be too small.
};
/*! \brief Shorthand for #AMkd_error_code enumeration.
 */
typedef enum AMkd_error_code AMkd_error_code;

/*! \brief Binary warning flags set in #AMkd_error_info structures.

    Describing non-critical errors, signaling suspicious, but correct data or redundancy of operations, etc. \n
    Zero indicating no warnings and consecutive bits (starting from the least significant) meaning different warnings. \n
    According to C language specification, maximum safe number of such flags is sixteen (not counting 0),
    as enum uses int type and int is guaranteed to be at least 16 bits long. \n
    Flags are to be set by OR-ing previous state of variable with new warning
    and to be checked by AND-ing the variable with a chosen flag.
 */
enum AMkd_warning_flags {
    AMKD_WARNING_NONE = 0, //!< No warnings.
    AMKD_WARNING_SURPLUS_SETTINGS = 0x1, //!< Warning: decoding settings present in encoded string and additionally provided by user; settings from string are used. \sa AMkd_decode
    AMKD_WARNING_MISSING_HEADER = 0x2, //!< Warning: no header to strip. \sa AMkd_strip_header
    AMKD_WARNING_CONTROL_CHARS = 0x4, //!< Warning: control characters present in decoded string (may suggest incorrect decoding).
    AMKD_WARNING_UNKNOWN_ENCODING = 0x8 //!< Warning: encoding impossible to detect. \sa AMkd_detect_encoding
};
/*! \brief Shorthand for #AMkd_warning_flags enumeration.
 */
typedef enum AMkd_warning_flags AMkd_warning_flags;

/*! \brief Return code for signaling successful execution or failure of functions supporting error handling.

    Only two values are possible:
    \li #AMKD_FAILURE is returned if any critical error has occured (and, in result, #AMkd_error_info.error_code
    does not equals #AMKD_ERROR_NONE). \n
    \li #AMKD_SUCCESS is returned otherwise (but still, there can be some warnings set in #AMkd_error_info.warning_flags).
 */
enum AMkd_error_indicator {
    AMKD_SUCCESS = 0, //!< No error.
    AMKD_FAILURE = 1 //!< Error occured.
};
/*! \brief Shorthand for #AMkd_error_indicator enumeration.
 */
typedef enum AMkd_error_indicator AMkd_error_indicator;


/*! \brief Structure containing data about function execution: encountered errors and warnings.

    To be passed as an argument to functions supporting error handling.
 */
struct AMkd_error_info {
    AMkd_error_code error_code; //!< Appropriate error code (or #AMKD_ERROR_NONE on successful execution). \sa AMkd_error_code
    AMkd_warning_flags warning_flags; //!< Binary warning flag register. \sa AMkd_warning_flags
};
/*! \brief Shorthand for AMkd_error_info structure.
 */
typedef struct AMkd_error_info AMkd_error_info;

/*! \brief Structure containing encoding/decoding settings.
 */
struct AMkd_config {
    int step_count; //!< Count of encoding steps (essential for correct decoding). The game engine casts this value to unsigned type, but for sanity's sake AMkd library takes its absolute value. Value of 0 cannot be used in functions.
    char letter; //!< Not used. Additional parameter provided by Aidem Media for indicating need of decoding script by the game engine.
};
/*! \brief Shorthand for AMkd_config structure.
 */
typedef struct AMkd_config AMkd_config;

/*! \brief Default encoding/decoding settings.

    Will work for any script taken from AM's games.
*/
const AMkd_config AMKD_CONFIG_DEFAULT;

/*! \brief No encoding settings.

    Indicates no decoding and cannot be used as standalone decoding/encoding settings.
*/
const AMkd_config AMKD_CONFIG_NONE;


/*! \brief Decodes \a encoded_str and places it at \a *decoded_str.

    \param encoded_str Encoded null-terminated string.
    \param decoded_str Address of decoded null-terminated string "returned" by function.
    \param settings Decoding settings. They have higher priority than (optional) header of \a encoded_str (except for 0-step settings). For defaults see #AMKD_CONFIG_DEFAULT
    \param error_info Pointer to structure used for storing error and warning data. May be NULL (only the return code is used then).

    \return Successful execution indicator.

    \warning Allocates memory for the decoded string: \a *decoded_str has to be manually freed after successful function execution.
    On failure, function is guaranteed to clean up before returning. \sa AMkd_deallocate_result
 */
AMkd_error_indicator AMkd_decode(const char *encoded_str, char **decoded_str, AMkd_config settings, AMkd_error_info *error_info);

/*! \brief Encodes \a decoded_str and places it at \a *encoded_str.

    \param decoded_str Decoded null-terminated string.
    \param encoded_str Address of encoded null-terminated string "returned" by function.
    \param settings Encoding settings. For defaults see #AMKD_CONFIG_DEFAULT
    \param error_info Pointer to structure used for storing error and warning data. May be NULL (only the return code is used then).

    \return Successful execution indicator.

    \warning Allocates memory for the encoded string: \a *encoded_str has to be manually freed after successful function execution.
    On failure, function is guaranteed to clean up before returning. \sa AMkd_deallocate_result
 */
AMkd_error_indicator AMkd_encode(const char *decoded_str, char **encoded_str, AMkd_config settings, AMkd_error_info *error_info);

/*! \brief Releases memory allocated by #AMkd_decode or #AMkd_encode.

    Provided to obviate the ambiguity concerning deallocation method.

    \param result_str Decoded or encoded string "returned" by #AMkd_decode or #AMkd_encode.
 */
void AMkd_deallocate_result(char *result_str);

/*! \brief Strips header from \a *encoded_str (if present).

    \param encoded_str Address of encoded null-terminated string.
    \param error_info Pointer to structure used for storing error and warning data. May be NULL (only the return code is used then).

    \return Successful execution indicator.
 */
AMkd_error_indicator AMkd_strip_header(char *encoded_str, AMkd_error_info *error_info);

/*! \brief Tries to detect encoding in \a encoded_str and writes it into \a *settings.

    For no or unknown encoding, the provided structure becomes equal to #AMKD_CONFIG_NONE. \n
    Also, unknown encoding is indicated by #AMKD_WARNING_UNKNOWN_ENCODING return code.

    \param encoded_str Encoded null-terminated string.
    \param settings Address of config structure. Must be provided by user.
    \param error_info Pointer to structure for saving error and warning data. May be NULL (only the return code is used then).

    \return Successful execution indicator.

    \todo Add more checks for better detection.
 */
AMkd_error_indicator AMkd_detect_encoding(const char *encoded_str, AMkd_config *settings, AMkd_error_info *error_info);
