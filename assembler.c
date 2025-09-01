/*
    Name 1: Andy Shen
    Name 2: Jeremy Wei
    UTEID 1: as224237
    UTEID 2: jw63339
*/

#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_LINE_LENGTH 255
#define MAX_OPCODE_LENGTH 10
#define MAX_SYMBOL_LENGTH 20

typedef struct st_entry {
    char symbol[MAX_SYMBOL_LENGTH];
    uint16_t addy;
    struct st_entry *next;
} st_entry;

enum {
    DONE,
    OK,
    EMPTY_LINE,
    INVALID_OPCODE
};

FILE *infile = NULL;
FILE *outfile = NULL;

st_entry *SYM_TABLE = NULL;

const char *const valid_opcodes[] = {
    "add",
    "and",
    "br",
    "brn",
    "brz",
    "brp",
    "brzp",
    "brnp",
    "brnz",
    "brnzp",
    "halt",
    "jmp",
    "jsr",
    "jsrr",
    "ldb",
    "ldw",
    "lea",
    "nop",
    "not",
    "ret",
    "lshf",
    "rshfl",
    "rshfa",
    "rti",
    "stb",
    "stw",
    "trap",
    "xor"};
const int num_valid_opcodes = sizeof(valid_opcodes) / sizeof(const char *const);

const char *const valid_pseudo_ops[] = {
    ".orig",
    ".end",
    ".fill"};
const int num_valid_pseudo_ops = sizeof(valid_pseudo_ops) / sizeof(const char *const);

st_entry *new_sym_entry(char *sym_name, uint16_t addr);
int generate_symbol_table(FILE *infile);
void print_sym_table();
int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode, char **pArg1, char **pArg2, char **pArg3, char **pArg4);
int isOpcode(char *in);
int toNum(char *pStr);

int main(int argc, char *argv[]) {
    struct stat in_info, out_info;

    if (argc != 3) { /* Check program usage */
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (stat(argv[1], &in_info) != 0) { /* Check if input file exists */
        fprintf(stderr, "Input file: %s, does not exist!\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if (stat(argv[2], &out_info) == 0) { /* Check if output file already exists */
        fprintf(stderr, "Output file: %s, already exists!\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    /* Open files */
    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");

    if (!infile) {
        fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if (!outfile) {
        fprintf(stderr, "Error: Cannot open file %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    generate_symbol_table(infile);
    print_sym_table();

    exit(EXIT_SUCCESS);
    fseek(infile, 0, SEEK_SET);
    char line_buff[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
    int status;

    do {
        status = readAndParse(infile, line_buff, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
        if (status == INVALID_OPCODE) {  // Check if the parsed output has a valid opcode
            exit(EXIT_FAILURE);
        }

        if (status != DONE && status != EMPTY_LINE) {
            printf("Line: %s\n\tLabel:  %s\n\tOpcode: %s\n\tArg1: %s\n\tArg2: %s\n\tArg3: %s\n\tArg4: %s\n", line_buff, lLabel, lOpcode, lArg1, lArg2, lArg3, lArg4);
        }
    } while (status != DONE);

    fclose(infile);
    fclose(outfile);
}

int generate_symbol_table(FILE *infile) {
    char line_buff[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
    int status;
    uint16_t cur_address = 0x0;

    do {
        status = readAndParse(infile, line_buff, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);

        if (cur_address == 0x0) {
            if (strncmp(lOpcode, ".orig", 5) == 0) {
                cur_address = toNum(lArg1);
                continue;
            }
        }

        if (strncmp(lOpcode, ".end", 4) == 0) {
            return EXIT_SUCCESS;
        }

        if (status == DONE || status == EMPTY_LINE) {
            continue;
        }

        if (*lLabel == '\0') {
            if (*lOpcode != '\0') cur_address += 2;
            continue;
        }

        /* Potentially detected new symbol. Compare with existing symbols and append if good */
        if (SYM_TABLE == NULL) {
            SYM_TABLE = new_sym_entry(lLabel, cur_address);
            cur_address += 2;
            continue;
        }

        /* Traverse LL */
        st_entry *pST_entry = SYM_TABLE;
        while (pST_entry->next != NULL) {
            if (strcmp(lLabel, pST_entry->symbol) == 0) {
                fprintf(stderr, "Label: %s, is already defined at address %04x.\n", lLabel, pST_entry->addy);
                return EXIT_FAILURE;
            }
            pST_entry = pST_entry->next;
        }

        /* Add entry */
        pST_entry->next = new_sym_entry(lLabel, cur_address);
        cur_address += 2;
    } while (status != DONE);
}

st_entry *new_sym_entry(char *sym_name, uint16_t addr) {
    st_entry *new_entry = (st_entry *)calloc(1, sizeof(st_entry));
    strncpy(new_entry->symbol, sym_name, MAX_SYMBOL_LENGTH * sizeof(char));
    new_entry->addy = addr;
    return new_entry;
}

void print_sym_table() {
    st_entry *pST_entry = SYM_TABLE;
    while (pST_entry != NULL) {
        printf("ADR: 0x%04x\tSYM: %s\n", pST_entry->addy, pST_entry->symbol);
        pST_entry = pST_entry->next;
    }
}

int isOpcode(char *in) {
    for (int i = 0; i < num_valid_opcodes; i++) {
        if (strncmp(in, valid_opcodes[i], MAX_OPCODE_LENGTH) == 0) {
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode, char **pArg1, char **pArg2, char **pArg3, char **pArg4) {
    char *lRet, *lPtr;
    int i;
    if (!fgets(pLine, MAX_LINE_LENGTH, pInfile))
        return (DONE);
    for (i = 0; i < strlen(pLine); i++)
        pLine[i] = tolower(pLine[i]);

    /* convert entire line to lowercase */
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

    /* ignore the comments */
    lPtr = pLine;

    while (*lPtr != ';' && *lPtr != '\0' && *lPtr != '\n')
        lPtr++;

    *lPtr = '\0';
    if (!(lPtr = strtok(pLine, "\t\n ,")))
        return (EMPTY_LINE);

    if (isOpcode(lPtr) != EXIT_SUCCESS && lPtr[0] != '.') /* found a label */
    {
        *pLabel = lPtr;
        if (!(lPtr = strtok(NULL, "\t\n ,")))
            return (OK);
    }

    *pOpcode = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return (OK);

    *pArg1 = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return (OK);

    *pArg2 = lPtr;
    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return (OK);

    *pArg3 = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,")))
        return (OK);

    *pArg4 = lPtr;

    return (OK);
}

int toNum(char *pStr) {
    char *t_ptr;
    char *orig_pStr;
    int t_length, k;
    int lNum, lNeg = 0;
    long int lNumLong;

    orig_pStr = pStr;
    if (*pStr == '#') /* decimal */
    {
        pStr++;
        if (*pStr == '-') /* dec is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for (k = 0; k < t_length; k++) {
            if (!isdigit(*t_ptr)) {
                printf("Error: invalid decimal operand, %s\n", orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg)
            lNum = -lNum;

        return lNum;
    } else if (*pStr == 'x') /* hex     */
    {
        pStr++;
        if (*pStr == '-') /* hex is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for (k = 0; k < t_length; k++) {
            if (!isxdigit(*t_ptr)) {
                printf("Error: invalid hex operand, %s\n", orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16); /* convert hex string into integer */
        lNum = (lNumLong > INT_MAX) ? INT_MAX : lNumLong;
        if (lNeg)
            lNum = -lNum;
        return lNum;
    } else {
        printf("Error: invalid operand, %s\n", orig_pStr);
        exit(4); /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}