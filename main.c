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

void cod(FILE *input, FILE *output, short var) {
    /*int optr, //output pointer
        iptr = 0, //input pointer
        len = strlen(input); //input length
    short step = 0, //current cipher step
        ecnt = 0; //count of "<E>"
    bool crlf = true;
    sprintf(output, "{<C:%hu>}\n", var);
    optr = strlen(output);
    if (len > 0) {
        if (!strstr(input, "\r\n")) crlf = false;
        while (iptr < len) {
            if (input[iptr] == '\r') {
                output[optr] = '\0';
                strcat(output, "<E>");
                ecnt++;
                iptr++;
                optr += 3;
            } else if (input[iptr] == '\n') {
                output[optr] = '\0';
                strcat(output, "<E>");
                ecnt++;
                optr += 3;
            } else {
                step++;
                if (step > var) step = 1;
                short shift = step / 2 + step % 2;
                if (step % 2) shift *= -1;
                output[optr] = input[iptr] - shift;
                optr++;
            }
            if (ecnt > 5) {
                output[optr] = '\0';*/
                /*strcat(output, crlf ? "\r\n" : "\n");
                optr += crlf + 1;*/
                /*strcat(output, crlf ? "\r\n" : "\n");
                optr += crlf + 1;
                ecnt = 0;
            }
            iptr++;
        }
    }*/
}

void dec(FILE *input, FILE *output)
{
    short mvmnt;
    char direction; //c = subtract firstly, d = add firstly
    char buffer;
    fscanf(input, "{<%c:%hu>}", &direction, &mvmnt);
    fread(&buffer, 1, 1, input);
    if (buffer == '\r') {
        fread(&buffer, 1, 1, input);
        if (buffer == '\n') {
            fread(&buffer, 1, 1, input);
        } else {
            fputs("Macintosh line-ending (CR) is unsupported!\n", stdout);
            return;
        }
    } else if (buffer == '\n') {
        fread(&buffer, 1, 1, input);
    }
    short step = 0, shift;
    while (fread(&buffer, 1, 1, input)) {
        if (buffer == '<') {
            fread(&buffer, 1, 1, input);
            if (buffer == 'E') {
                fread(&buffer, 1, 1, input);
                if (buffer == '>') {
                    fputc('\n', output);
                } else {
                    step++;
                    if (step > mvmnt) step = 1;
                    shift = step / 2 + step % 2;
                    if (step % 2) shift *= -1;
                    fputc('<' + shift, output);
                    step++;
                    if (step > mvmnt) step = 1;
                    shift = step / 2 + step % 2;
                    if (step % 2) shift *= -1;
                    fputc('E' + shift, output);
                    step++;
                    if (step > mvmnt) step = 1;
                    shift = step / 2 + step % 2;
                    if (step % 2) shift *= -1;
                    fputc(buffer + shift, output);
                }
            } else {
                step++;
                if (step > mvmnt) step = 1;
                shift = step / 2 + step % 2;
                if (step % 2) shift *= -1;
                fputc('<' + shift, output);
                step++;
                if (step > mvmnt) step = 1;
                shift = step / 2 + step % 2;
                if (step % 2) shift *= -1;
                fputc(buffer + shift, output);
            }
        } else if (buffer == '\r' || buffer == '\n') {
        } else {
            step++;
            if (step > mvmnt) step = 1;
            shift = step / 2 + step % 2;
            if (step % 2) shift *= -1;
            fputc(buffer + shift, output);
        }
    }
}

int main(int argc, char **argv)
{
    bool fp = false, //fp ? plik : konsola
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
            if (argc == 2) fp = false;
            else fp = true;
            if (argv[1][1] == 'd') decode = true;
            else if (argv[1][1] == 'e' || argv[1][1] == 'k') decode = false;
            else {
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
            src = fopen(argv[i], "rb");
            if (!src) {
                fprintf(stderr, "Nieprawidlowa nazwa pliku: %s\n", argv[i]);
                return 0;
            }
            if (!decode) { //code
                char newname[strlen(argv[i]) + 5];
                strcpy(newname, argv[i]);
                strcat(newname, ".kod");
                fnew = fopen(newname, "wb");
                cod(src, fnew, 6);
                fclose(fnew);
            } else { //decode
                char newname[strlen(argv[i]) + 5];
                strcpy(newname, argv[i]);
                strcat(newname, ".dek");
                fnew = fopen(newname, "wb");
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
