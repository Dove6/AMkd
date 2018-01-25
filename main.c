#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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

void cod(char *input, char *output, short var) {
    int optr, //output pointer
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
                output[optr] = '\0';
                /*strcat(output, crlf ? "\r\n" : "\n");
                optr += crlf + 1;*/
                strcat(output, crlf ? "\r\n" : "\n");
                optr += crlf + 1;
                ecnt = 0;
            }
            iptr++;
        }
    }
}

void dec(char *input, char *output)
{
    short mvmnt;
    int iptr, //input pointer
        optr = 0, //output pointer
        len = strlen(input);
    bool crlf = true;
    if (len >= 7) {
        sscanf(input, "{<C:%hu>}\n", &mvmnt);
        iptr = strchr(input, '}') - input + 1;
        if (input[iptr] == '\r') iptr += 2;
        else {
            crlf = false;
            iptr++;
        }
        short step = 0;
        while (iptr < len) {
            if (input[iptr] == '<' && input[iptr + 1] == 'E' && input[iptr + 2] == '>') {
                output[optr] = '\0';
                strcat(output, crlf ? "\r\n" : "\n");
                optr += crlf ? 2 : 1;
                iptr += 2;
            } else if (input[iptr] == '\r');
            else if (input[iptr] == '\n');
            else {
                step++;
                if (step > mvmnt) step = 1;
                short shift = step / 2 + step % 2;
                if (step % 2) shift *= -1;
                output[optr] = input[iptr] + shift;
                optr++;
            }
            iptr++;
        }
    }
}

int main(int argc, char **argv)
{
    char *buffer, //console input buffer
        *input, //main input buffer
        *output; //main output buffer
    int fsize, //file size
        ip = 0, //console input pointer
        len; //length of buffer
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
            fseek(src, 0, SEEK_END);
            fsize = ftell(src);
            rewind(src);
            if (fsize < BUFSIZE) {
                input = (char *)malloc(BUFSIZE);
                fread(input, fsize, 1, src);
                fclose(src);
                if (!decode) { //code
                    output = (char *)malloc(BUFSIZE * 1.5);
                    cod(input, output, 6);
                    char newname[strlen(argv[i]) + 5];
                    strcpy(newname, argv[i]);
                    strcat(newname, ".kod");
                    fnew = fopen(newname, "wb");
                    len = strlen(output);
                    fwrite(output, len, 1, fnew);
                    fclose(fnew);
                } else { //decode
                    output = (char *)malloc(BUFSIZE);
                    dec(input, output);
                    char newname[strlen(argv[i]) + 5];
                    strcpy(newname, argv[i]);
                    strcat(newname, ".dek");
                    fnew = fopen(newname, "wb");
                    len = strlen(output);
                    fwrite(output, len, 1, fnew);
                    fclose(fnew);
                }
                free(input);
                free(output);
            } else {
                fclose(src);
                fprintf(stderr, "Blad wejscia - plik zbyt duzy: %s\n", argv[i]);
                return 0;
            }
        }
    } else {
        input = (char *)malloc(BUFSIZE);
        buffer = (char *)malloc(BUFSIZE);
        *buffer = 0;
        *input = 0;
        while (ip < BUFSIZE - 1) {
            if (fgets(buffer, BUFSIZE - 1 - strlen(input), stdin) != NULL) {
                strcat(input, buffer);
                ip = strlen(input);
            } else break;
        }
        if (strlen(input) == 0) {
            fputs("Blad wejscia: brak wejscia\n", stderr);
        } else {
            if (!decode) {
                output = (char *)malloc(BUFSIZE * 1.5);
                cod(input, output, 6);
            } else {
                output = (char *)malloc(BUFSIZE);
                dec(input, output);
            }
            puts(output);
            free(output);
        }
        free(input);
    }

    return 0;
}
