#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//TODO: get rid of buffer and process on-time
int BUFSIZE = 16777216;

#ifdef _WIN32
char help[] = "Witaj w programie do kodowania oraz dekodowania szyfru Aidem Media!\n\n\
            Uzycie: AMkd [operacja] [plik]\n\n\
            Dostepne operacje:\n\
            /d\t\tdekoduje ciag tekstowy; opcja domyslna\n\
            /e, /k\tkoduje ciag tekstowy\n\n\
            Program uruchomiony bez argumentow oczekuje na ciag znakow zakonczony EOF (^Z)\n\n\
            Wynik operacji na pliku zapisywany jest w nowym pliku z dodatkowym\n\
            \trozszerzeniem, odpowiednio .dek lub .kod.\n";
#else
char help[] = "Witaj w programie do kodowania oraz dekodowania szyfru Aidem Media!\n\n\
            Uzycie: AMkd [operacja] [plik]\n\n\
            Dostepne operacje:\n\
            -d\t\tdekoduje ciag tekstowy; opcja domyslna\n\
            -e, -k\tkoduje ciag tekstowy\n\n\
            Program uruchomiony bez argumentow oczekuje na ciag znakow zakonczony EOF (^D)\n\n\
            Wynik operacji na pliku zapisywany jest w nowym pliku z dodatkowym\n\
            \trozszerzeniem, odpowiednio .dek lub .kod.\n";
#endif

void print_help(void)
{
    puts(help);
}

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

//TODO: support for probable reverse cipher ({<D:%hu>})
void cod(FILE *input, FILE *output, unsigned short var) {
    unsigned short step = 0, //current cipher step
                   ecnt = 0; //count of "<E>"
    short shift = 0;
    char buffer;
    fprintf(output, "{<C:%hu>}\n", var);
    fpos_t prev_pos;
    fgetpos(input, &prev_pos);
    while (fread(&buffer, 1, 1, input)) {
        if (buffer == '\r') {
            fread(&buffer, 1, 1, input);
            if (buffer != '\n') {
                fsetpos(input, &prev_pos);
            }
            fputs("<E>", output);
            ecnt++;
        } else if (buffer == '\n') {
            fputs("<E>", output);
            ecnt++;
        } else {
            fputc(buffer - calc_shift(&step, &shift, var), output);
            fgetpos(input, &prev_pos);
        }
        if (ecnt > 5) {
            fputc('\n', output);
            ecnt = 0;
        }
    }
    fputc('\n', output);
}

void dec(FILE *input, FILE *output)
{
    unsigned short mvmnt,
                   step = 0;
    short shift = 0,
          direction_multiplier = 1;
    char direction,
         buffer;
    fpos_t prev_pos;
    fscanf(input, "{<%c:%hu>}", &direction, &mvmnt);
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
            printf("Error: Unknown file format in file %s\n", input->_tmpfname);
            return;
        }
    }
    while (fread(&buffer, 1, 1, input)) {
        if (buffer == '<') {
            fgetpos(input, &prev_pos);
            char e_buffer[2];
            fread(&e_buffer, 1, 2, input);
            if (e_buffer[0] == 'E' && e_buffer[1] == '>') {
                fputc('\n', output);
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

int main(int argc, char **argv)
{
    bool fp = false, //fp ? file : command line
         decode = true, //decode ? dec() : cod()
         cdprm = false; //codec parameter indicator
    FILE *fnew, //output file
         *src; //source file
    if (argc > 1) {
        #ifdef _WIN32
        if (argv[1][0] == '/') {
        #else
        if (argv[1][0] == '-') {
        #endif
            cdprm = true;
            if (argc == 2) {
                fp = false;
            } else {
                fp = true;
            }
            if (argv[1][1] == 'd') {
                decode = true;
            } else if (argv[1][1] == 'e' || argv[1][1] == 'k') {
                decode = false;
            } else {
                print_help();
                return 0;
            }
        } else {
            cdprm = false;
            decode = true;
            fp = true;
        }
    }

    if (fp) {
        for (short i = 1 + cdprm; i < argc; i++) {
            #ifdef _WIN32
            if (argv[i][0] == '/') {
            #else
            if (argv[i][0] == '-') {
            #endif
                fputs("Dlaczego?", stderr);
                print_help();
                return 0;
            }
            src = fopen(argv[i], "r");
            if (!src) {
                fprintf(stderr, "Nieprawidlowa nazwa pliku: %s\n", argv[i]);
                return 0;
            }
            if (!decode) { //code
                char newname[strlen(argv[i]) + 5];
                strcpy(newname, argv[i]);
                strcat(newname, ".kod");
                fnew = fopen(newname, "w");
                cod(src, fnew, 6);
                fclose(fnew);
            } else { //decode
                char newname[strlen(argv[i]) + 5];
                strcpy(newname, argv[i]);
                strcat(newname, ".dek");
                fnew = fopen(newname, "w");
                dec(src, fnew);
                fclose(fnew);
            }
            fclose(src);
        }
    } else {
        if (!decode) {
            cod(stdin, stdout, 6);
        } else {
            dec(stdin, stdout);
        }
    }

    return 0;
}
