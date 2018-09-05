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

int calc_shift(short *step, short *shift, short mvmnt)
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

void cod(FILE *input, FILE *output, short var) {
    short step = 0, //current cipher step
          shift = 0,
          ecnt = 0; //count of "<E>"
    char buffer;
    fprintf(output, "{<C:%hu>}\n", var);
    fpos_t prev_pos;
    fgetpos(input, &prev_pos);
    while (fread(&buffer, 1, 1, input)) {
        if (buffer == '\r') {
            fread(&buffer, 1, 1, input);
            if (buffer == '\n') {
                fread(&buffer, 1, 1, input);
            }
            fputs("<E>", output);
            ecnt++;
        } else if (buffer == '\n') {
            fread(&buffer, 1, 1, input);
            fputs("<E>", output);
            ecnt++;
        } else {

        }
        if (ecnt > 5) {
            fputc('\n', output);
            ecnt = 0;
        }
        fputc(buffer - calc_shift(&step, &shift, var), output);
        if (buffer - shift == '\a') {
            fprintf(output, "\n\nDZIWNY ZNAK: %c (shift: -%hu)\n\n", buffer, shift);
        }
    }
}

void dec(FILE *input, FILE *output)
{
    short mvmnt,
          step = 0,
          shift = 0;
    //TODO: support for probable reverse cipher ({<D:%hu>}) - "direction" variable
    char direction, //c = subtraction first, d = addition first
         buffer;
    fscanf(input, "{<%c:%hu>}", &direction, &mvmnt);
    while (fread(&buffer, 1, 1, input)) {
        if (buffer == '<') {
            fread(&buffer, 1, 1, input);
            if (buffer == 'E') {
                fread(&buffer, 1, 1, input);
                if (buffer == '>') {
                    fputc('\n', output);
                } else {
                    char queue[3] = {'<', 'E', buffer};
                    for (int i = 0; i < 3; i++) {
                        fputc(queue[i] + calc_shift(&step, &shift, mvmnt), output);
                    }
                }
            } else {
                char queue[2] = {'<', buffer};
                for (int i = 0; i < 2; i++) {
                    fputc(queue[i] + calc_shift(&step, &shift, mvmnt), output);
                }
            }
        } else if (buffer == '\r' || buffer == '\n') {
        } else {
            fputc(buffer + calc_shift(&step, &shift, mvmnt), output);
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
