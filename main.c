#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int BUFSIZE = 16777216;

void print_help(void)
{
    puts("Witaj w programie do kodowania oraz dekodowania szyfru Aidem Media!\n\n\
         Uzycie: AM_kodek [operacja] [plik]\n\n\
         Dostepne operacje:\n\
         -d\t\tdekoduje ciag tekstowy; opcja domyslna\n\
         -e, -k\tkoduje ciag tekstowy\n\n\
         Program uruchomiony bez argumentow oczekuje na ciag znakow zakonczony EOF:\n\
         ^Z\t\tdla Windows\n\
         ^D\t\tdla *nix\n\n\
         Wynik operacji na pliku zapisywany jest w nowym pliku z dodatkowym\n\
         \trozszerzeniem, odpowiednio .dek lub .kod.\n");
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
        decode = true; //decode ? dec() : cod()
    FILE *fnew, //output file
        *src; //source file
    if (argc > 3) {
        print_help();
        return 0;
    } else if (argc == 3) { //operacja + plik
        fp = true;
        if (argv[1][1] == 'd');// decode = true;
        else if (argv[1][1] == 'e' || argv[1][1] == 'k') {
            decode = false;
        } else {
            print_help();
            return 0;
        }
        src = fopen(argv[2], "rb");
        if (!src) {
            fprintf(stderr, "Nieprawidlowa nazwa pliku: %s\n", argv[2]);
            return 0;
        }
        fseek(src, 0, SEEK_END);
        fsize = ftell(src);
        rewind(src);
        if (fsize < BUFSIZE) {
            input = malloc(BUFSIZE);
            fread(input, fsize, 1, src);
            fclose(src);
            if (!decode) { //code
                output = malloc(BUFSIZE * 1.5);
                cod(input, output, 6);
                char newname[strlen(argv[2]) + 5];
                strcpy(newname, argv[2]);
                strcat(newname, ".kod");
                fnew = fopen(newname, "wb");
                len = strlen(output);
                fwrite(output, len, 1, fnew);
                fclose(fnew);
            } else { //decode
                output = malloc(BUFSIZE);
                dec(input, output);
                char newname[strlen(argv[2]) + 5];
                strcpy(newname, argv[2]);
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
            fputs("Blad wejscia: plik zbyt duzy", stderr);
            return 0;
        }
    } else if (argc == 2) { //operacja ^ plik
        if (argv[1][0] != '/' && argv[1][0] != '-') {
            fp = true;
            src = fopen(argv[1], "rb");
            if (!src) {
                fprintf(stderr, "Nieprawidlowa nazwa pliku: %s\n", argv[1]);
                return 0;
            }
            fseek(src, 0, SEEK_END);
            fsize = ftell(src);
            rewind(src);
            if (fsize < BUFSIZE) {
                input = malloc(BUFSIZE);
                fread(input, fsize, 1, src);
                fclose(src);
                if (!decode) { //code
                    output = malloc(BUFSIZE * 1.5);
                    cod(input, output, 6);
                    char newname[strlen(argv[1]) + 5];
                    strcpy(newname, argv[1]);
                    strcat(newname, ".kod");
                    fnew = fopen(newname, "wb");
                    len = strlen(output);
                    fwrite(output, len, 1, fnew);
                    fclose(fnew);
                } else { //decode
                    output = malloc(BUFSIZE);
                    dec(input, output);
                    char newname[strlen(argv[1]) + 5];
                    strcpy(newname, argv[1]);
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
                fputs("Blad wejscia: plik zbyt duzy", stderr);
                return 0;
            }
        } else {
            if (argv[1][1] == 'd');
            else if (argv[1][1] == 'e' || argv[1][1] == 'k') decode = false;
            input = malloc(BUFSIZE);
            buffer = malloc(BUFSIZE);
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
                    output = malloc(BUFSIZE * 1.5);
                    cod(input, output, 6);
                } else {
                    output = malloc(BUFSIZE);
                    dec(input, output);
                }
                puts(output);
                free(output);
            }
            free(input);
        }
    } else { //tryb interaktywny
        input = malloc(BUFSIZE);
        buffer = malloc(BUFSIZE);
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
            output = malloc(BUFSIZE);
            cod(input, output, 6);
            puts(output);
            free(output);
        }
        free(input);
    }
    return 0;
}
