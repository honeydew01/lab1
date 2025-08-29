#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

int main(int argc, char* argv[]) {
    char *prgName   = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;

    prgName   = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];

    printf("program name = '%s'\n", prgName);
    printf("input file name = '%s'\n", iFileName);
    printf("output file name = '%s'\n", oFileName);

    /* open the source file */
    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");
        
    if (!infile) {
    printf("Error: Cannot open file %s\n", argv[1]);
    exit(4);
        }
    if (!outfile) {
    printf("Error: Cannot open file %s\n", argv[2]);
    exit(4);
    }

    /* Do stuff with files */

    fclose(infile);
    fclose(outfile);
    }
