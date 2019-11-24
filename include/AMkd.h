/*! \file AMkd.h
    \brief AMkd library header file.

    Provides documented function declarations for library users.
 */


/*! \brief Error codes returned by library functions.

    Warnings are negative numbers.
    Errors are positive numbers.
 */
enum AMkd_error {
    AMKD_ERROR_NONE = 0, //!< No error
    AMKD_WARNING_SURPLUS_SETTINGS = -255, //!< Warning: decoding settings present in encoded string and additionally provided by user; settings from string are used \sa AMkd_decode
    AMKD_WARNING_MISSING_HEADER, //!< Warning: no header to strip \sa AMkd_strip_header
    AMKD_WARNING_CONTROL_CHARS, //!< Warning: control characters present in decoded string (may suggest incorrect decoding)
    AMKD_ERROR_MISSING_SETTINGS = 1, //!< Error: no settings provided for encoding/decoding
    AMKD_ERROR_OUT_OF_MEMORY //!< Error: no memory available for allocation for output string
};
/*! \brief Shorthand for #AMkd_error enumeration.
 */
typedef enum AMkd_error AMkd_error;


/*! \brief Structure containing encoding/decoding settings.
 */
struct AMkd_config {
    int step_count; //!< Count of encoding steps (essential for correct decoding)
    char letter; //!< Not used; additional parameter provided by Aidem Media for indicating need of decoding script by the game engine
};
/*! \brief Shorthand for AMkd_config structure.
 */
typedef struct AMkd_config AMkd_config;

/*! \brief Default encoding/decoding settings.

    Will work for any script taken from AM's games.
*/
AMkd_config AMKD_CONFIG_DEFAULT = {6, 'C'};

/*! \brief No encoding settings.

    Makes string pass without changes.
*/
AMkd_config AMKD_CONFIG_NONE = {0, 'C'};


/*! \brief Decodes \a encoded_str and places it at \a *decoded_str.

    \param encoded_str Encoded null-terminated string.
    \param decoded_str Address of decoded null-terminated string "returned" by function.
    \param settings Decoding settings. If \a encoded_str has a header, they are not used. For defaults see #AMKD_CONFIG_DEFAULT

    \warning Allocates memory for the decoded string: \a *decoded_str has to be manually freed.
 */
AMkd_error AMkd_decode(const char *encoded_str, char **decoded_str, AMkd_config settings);

/*! \brief Encodes \a decoded_str and places it at \a *encoded_str.

    \param decoded_str Decoded null-terminated string.
    \param encoded_str Address of encoded null-terminated string "returned" by function.
    \param settings Encoding settings. For defaults see #AMKD_CONFIG_DEFAULT

    \warning Allocates memory for the encoded string: \a *encoded_str has to be manually freed.
 */
AMkd_error AMkd_encode(const char *decoded_str, char **encoded_str, AMkd_config settings);

/*! \brief Strips header from \a *encoded_str (if present).

    \param encoded_str Address of encoded null-terminated string.
 */
AMkd_error AMkd_strip_header(char **encoded_str);

/*! \brief Tries to detect encoding in \a encoded_str and writes it into \a *settings.

    For no or unknown encoding, the provided structure becomes equal to #AMKD_CONFIG_NONE

    \param encoded_str Encoded null-terminated string.
    \param settings Address of config structure. Must be provided by user.
 */
AMkd_error AMkd_detect_encoding(const char *encoded_str, AMkd_config *settings);