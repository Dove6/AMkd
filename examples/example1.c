#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "AMkd.h"

#ifdef _WIN32
char help[] = "Witaj w programie do kodowania oraz dekodowania szyfru Aidem Media!\n\n"
            "Uzycie: AMkd [operacja] [plik]\n\n"
            "Dostepne operacje:\n"
            "/d\tdekoduje ciag tekstowy; opcja domyslna\n"
            "/e, /k\tkoduje ciag tekstowy\n\n"
            "Program uruchomiony bez argumentow oczekuje na ciag znakow zakonczony EOF (^Z)\n\n"
            "Wynik operacji na pliku zapisywany jest w nowym pliku z dodatkowym\n"
            "\trozszerzeniem, odpowiednio .dek lub .kod.";
#else
char help[] = "Witaj w programie do kodowania oraz dekodowania szyfru Aidem Media!\n\n"
            "Uzycie: AMkd [operacja] [pliki]\n\n"
            "Dostepne operacje:\n"
            "-d\tdekoduje ciag tekstowy; opcja domyslna\n"
            "-e, -k\tkoduje ciag tekstowy\n\n"
            "Program uruchomiony bez argumentow oczekuje na ciag znakow zakonczony EOF (^D)\n\n"
            "Wynik operacji na pliku zapisywany jest w nowym pliku z dodatkowym\n"
            "\trozszerzeniem, odpowiednio .dek lub .kod.";
#endif

/**
    @brief Prints help text (global 'help' variable).
*/
void print_help(void)
{
    puts(help);
}

/**
    Sample shift chain for mvmnt = 6: -1, 1, -2, 2, -3, 3
    @brief Calculate ASCII shift value for current character.

    @param @a step - pointer to variable holding previous step value of cipher
    @param @a shift - pointer to variable holding previous shift value
    @param @a mvmnt - upper limit of step value

    @return current shift value
*/
int calc_shift(unsigned short *step, short *shift, unsigned short mvmnt)
{
    (*step)++;
    if (*step > mvmnt) {
        *step = 1;
    }
    *shift = *step / 2 + *step % 2;
    if (*step % 2) {
        *shift *= -1;
    }
    return *shift;
}

/**
    @brief Ciphers 'input' file contents and saves it in the 'output' file.

    @param @a input - pointer to open input FILE stream
    @param @a output - pointer to open output FILE stream
    @param @a var - upper limit of step value
    @param @a reverse - boolean indicating if cipher shift is negated
*/
void cod(FILE *input, FILE *output, unsigned short var, bool reverse) {
    unsigned short step = 0, //current cipher step
                   ecnt = 0; //count of '<E>' tags
    short shift = 0,
          direction_multiplier = 1;
    char buffer,
         direction = 'C';
    if (reverse) {
        direction_multiplier *= -1;
        direction = 'D';
    }
    //Prints header indicating cipher 'direction' and range
    fprintf(output, "{<%c:%hu>}\n", direction, var);
    fpos_t prev_pos;
    while (fread(&buffer, 1, 1, input)) {
        if (buffer == '\r') { //CR or CRLF line ending
            fgetpos(input, &prev_pos);
            fread(&buffer, 1, 1, input);
            //In case of CR line ending, goes back a character in order not to omit it
            if (buffer != '\n') {
                fsetpos(input, &prev_pos);
            }
            fputs("<E>", output);
            ecnt++;
        } else if (buffer == '\n') { //LF line ending
            fputs("<E>", output);
            ecnt++;
        } else {
            fputc(buffer - calc_shift(&step, &shift, var) * direction_multiplier, output);
        }
        //Makes line break every 6 '<E>' tags
        if (ecnt > 5) {
            fputc('\n', output);
            ecnt = 0;
        }
    }
    fputc('\n', output);
}

/**
    @brief Deciphers 'input' file contents and saves it in the 'output' file.

    @param @a input - pointer to open input FILE stream
    @param @a output - pointer to open output FILE stream
*/
void dec(FILE *input, FILE *output)
{
    unsigned short mvmnt,
                   step = 0;
    short shift = 0,
          direction_multiplier = 1;
    char direction,
         buffer;
    fpos_t prev_pos;
    if (fscanf(input, "{<%c:%hu>}", &direction, &mvmnt) != 2) {
        fprintf(stderr, "Blad czytania wejscia: %s\n", input->_tmpfname);
        return;
    }
    switch (direction) {
        case 'c':
        case 'C': { //subtraction first
            break;
        }
        case 'd':
        case 'D': { //addition first
            direction_multiplier *= -1;
            break;
        }
        default: {
            fprintf(stderr, "Nieznany format pliku: %s\n", input->_tmpfname);
            return;
        }
    }
    while (fread(&buffer, 1, 1, input)) {
        if (buffer == '<') {
            fgetpos(input, &prev_pos);
            char e_buffer[2];
            fread(&e_buffer, 1, 2, input);
            //In case of '<E>' tag, outputs a new line
            if (e_buffer[0] == 'E' && e_buffer[1] == '>') {
                fputc('\n', output);
            //otherwise, processes a '<' character and reverts file position pointer just after it
            } else {
                fsetpos(input, &prev_pos);
                fputc(buffer + calc_shift(&step, &shift, mvmnt) * direction_multiplier, output);
            }
        } else if (buffer == '\r' || buffer == '\n') {
        } else {
            fputc(buffer + calc_shift(&step, &shift, mvmnt) * direction_multiplier, output);
        }
    }
}

/**
    @brief Main function.

    @param @a argc - arguments' count
    @param @a argv - arguments' list
*/
int main(int argc, char **argv)
{
    struct {
        short file_count;
        bool decode;
    } options;
    options.file_count = 0;
    options.decode = true;
    FILE *input,
         *output;
    for (short i = 1; i < argc; i++) {
        #ifdef _WIN32
        if (argv[i][0] == '/') {
        #else
        if (argv[i][0] == '-') {
        #endif
            if (i == 1) {
                switch (argv[i][1]) {
                    case 'd': {
                        break;
                    }
                    case 'e':
                    case 'k': {
                        options.decode = false;
                        break;
                    }
                    #ifdef _WIN32
                    case '?':
                    #endif
                    case 'h': {
                        print_help();
                        return 0;
                    }
                }
            //Incorrect parameter order
            } else {
                fputs("Dlaczego?\n\n", stderr);
                print_help();
                return 0;
            }
        } else {
            options.file_count++;
            input = fopen(argv[i], "r");
            if (!input) {
                fprintf(stderr, "Nieprawidlowa nazwa pliku: %s\n", argv[i]);
                return 0;
            }
            char out_name[strlen(argv[i]) + 5];
            strcpy(out_name, argv[i]);
            strcat(out_name, options.decode ? ".dek" : ".kod");
            output = fopen(out_name, "w");
            if (!output) {
                fprintf(stderr, "Cos poszlo nie tak przy tworzeniu pliku: %s\n", out_name);
                return 0;
            }
            if (options.decode) {
                dec(input, output);
            } else {
                cod(input, output, 6, false);
            }
            fclose(output);
            fclose(input);
        }
    }

    if (options.file_count == 0) {
        if (options.decode) {
            dec(stdin, stdout);
        } else {
            cod(stdin, stdout, 6, false);
        }
    }

    return 0;
}
