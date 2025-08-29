#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#define MAX_LINE_LENGTH 255

int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4);

enum{
    DONE, OK, EMPTY_LINE
};

FILE *infile = NULL;
FILE *outfile = NULL;

int* labels = {"ADD", "AND", "BRn", "BRz", "BRp", "BR", "BRzp", "BRnp", "BRnz", "BRnzp", "HALT", "JMP", "JSR", "JSRR", "LDB", "LDW",
"LEA", "NOP", "NOT", "RET", "LSHF", "RSHFL", "RSHFA", "RTI", "STB", "STW", "TRAP", "XOR"};
int numLabels = 25;

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

int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4)
{
    char * lRet, * lPtr;
    int i;
    if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) )
    return( DONE );
    for( i = 0; i < strlen( pLine ); i++ )
    pLine[i] = tolower( pLine[i] );
    
        /* convert entire line to lowercase */
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

    /* ignore the comments */
    lPtr = pLine;

    while( *lPtr != ';' && *lPtr != '\0' &&
    *lPtr != '\n' ) 
    lPtr++;

    *lPtr = '\0';
    if( !(lPtr = strtok( pLine, "\t\n ," ) ) ) 
    return( EMPTY_LINE );

    if( isOpcode( lPtr ) == -1 && lPtr[0] != '.' ) /* found a label */
    {
    *pLabel = lPtr;
    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
    }
    
        *pOpcode = lPtr;

    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );
    
        *pArg1 = lPtr;
    
        if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg2 = lPtr;
    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg3 = lPtr;

    if( !( lPtr = strtok( NULL, "\t\n ," ) ) ) return( OK );

    *pArg4 = lPtr;

    return( OK );
}