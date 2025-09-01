/*
    Name 1: Andy Shen
    Name 2: Jeremy Wei
    UTEID 1: as224237
    UTEID 2: jw63339
*/

/* --------------------------------------------- Include defs ---------------------------------------------- */

#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* --------------------------------------------- Macro defs ------------------------------------------------ */

#define MAX_LINE_LENGTH 255
#define MAX_OPCODE_LENGTH 10
#define MAX_SYMBOL_LENGTH 20
#define MAX_REG_NAME_LENGTH 3

#define REG_NAMES \
    X(r0, 0)      \
    X(r1, 1)      \
    X(r2, 2)      \
    X(r3, 3)      \
    X(r4, 4)      \
    X(r5, 5)      \
    X(r6, 6)      \
    X(r7, 7)      \
    X(ssp, 6)     \
    X(usp, 6)

#define OPCODE_LIST \
    X(add)          \
    X(and)          \
    X(br)           \
    X(brn)          \
    X(brz)          \
    X(brp)          \
    X(brzp)         \
    X(brnp)         \
    X(brnz)         \
    X(brnzp)        \
    X(halt)         \
    X(jmp)          \
    X(jsr)          \
    X(jsrr)         \
    X(ldb)          \
    X(ldw)          \
    X(lea)          \
    X(nop)          \
    X(not)          \
    X(ret)          \
    X(lshf)         \
    X(rshfl)        \
    X(rshfa)        \
    X(rti)          \
    X(stb)          \
    X(stw)          \
    X(trap)         \
    X(xor)

#define OPCODE_FUNC_NAME(opcode_name) op2hex_##opcode_name
#define OPCODE_FUNC_SIGNATURE(opcode_name) uint16_t opcode_name(uint16_t cur_addr, char *opcode, char *arg1, char *arg2, char *arg3, char *arg4)
#define OPCODE_FUNC_PROTO(opcode_name) OPCODE_FUNC_SIGNATURE(OPCODE_FUNC_NAME(opcode_name))
#define OPCODE_FUNC_INIT(opcode_val) uint16_t ret_val = opcode_val << 12

/* --------------------------------------------- Type defs ------------------------------------------------- */

/* Return values for the readandparse function */
enum {
    DONE,
    OK,
    EMPTY_LINE
};

/* Struct to act as an entry in the symbol table where the symbol table is a linked list of entries */
typedef struct st_entry {
    char symbol[MAX_SYMBOL_LENGTH];
    uint16_t addy;
    struct st_entry *next;
} st_entry;

typedef struct opcode_funcs {
    const char *opcode;
    OPCODE_FUNC_SIGNATURE((*func));
} opcode_funcs;

typedef struct reg_associations {
    const char *reg_name;
    int reg_val;
} reg_associations;

/* --------------------------------------------- Func prototypes ------------------------------------------- */

int convert_to_hex(FILE *infile, FILE *outfile);
int generate_symbol_table(FILE *infile);
uint16_t proc_opcode(char *opcode, char *arg1, char *arg2, char *arg3, char *arg4);
st_entry *new_sym_entry(char *sym_name, uint16_t addr);
void print_sym_table();
int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode, char **pArg1, char **pArg2, char **pArg3, char **pArg4);
int isOpcode(char *in);
int toNum(char *pStr);
int regToNum(char *reg_name);
uint16_t offset_calc(uint16_t cur_addr, char* label_name, uint16_t num_bits);

#define X(OPCODE_NAME) OPCODE_FUNC_PROTO(OPCODE_NAME);
OPCODE_LIST
#undef X

/* --------------------------------------------- Constant defs --------------------------------------------- */

static const opcode_funcs opcodes_dispach_table[] = {
#define X(OPCODE_NAME) {#OPCODE_NAME, OPCODE_FUNC_NAME(OPCODE_NAME)},
    OPCODE_LIST
#undef X
};
const int num_valid_opcodes = sizeof(opcodes_dispach_table) / sizeof(opcode_funcs);

const char *const valid_pseudo_ops[] = {
    ".orig",
    ".end",
    ".fill"};
const int num_valid_pseudo_ops = sizeof(valid_pseudo_ops) / sizeof(const char *const);

static const reg_associations valid_reg_names[] = {
#define X(reg_name, reg_value) {#reg_name, reg_value},
    REG_NAMES
#undef X
};
const int num_valid_reg_names = sizeof(valid_reg_names) / sizeof(reg_associations);

/* --------------------------------------------- Global defs ----------------------------------------------- */

FILE *infile = NULL;
FILE *outfile = NULL;
st_entry *SYM_TABLE = NULL;

/* --------------------------------------------- Main ------------------------------------------------------ */

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

    int status = generate_symbol_table(infile);
    if (status != EXIT_SUCCESS) {
        exit(EXIT_FAILURE);
    }

    fseek(infile, 0, SEEK_SET);
    status = convert_to_hex(infile, outfile);
    if (status != EXIT_SUCCESS) {
        exit(EXIT_FAILURE);
    }

    fclose(infile);
    fclose(outfile);

    /* Clean up sym table */
    st_entry *pST_entry = SYM_TABLE;
    while (pST_entry != NULL) {
        st_entry *temp = pST_entry;
        pST_entry = temp->next;
        free(temp);
    }
}

/* --------------------------------------------- Func defs ------------------------------------------------- */

int convert_to_hex(FILE *infile, FILE *outfile) {
    char line_buff[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
    int status;
    uint16_t cur_address = 0;

    do {
        status = readAndParse(infile, line_buff, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);

        if (cur_address == 0x0) {
            if (strncmp(lOpcode, ".orig", 5) == 0) {
                cur_address = toNum(lArg1);
                fprintf(outfile, "0x%.04x\n", cur_address);
                continue;
            }
        }

        if (strncmp(lOpcode, ".end", 4) == 0) {
            return EXIT_SUCCESS;
        }

        if (status == DONE || status == EMPTY_LINE) {
            continue;
        }

        cur_address += 2;

        if (strncmp(lOpcode, ".fill", 5) == 0) {
            fprintf(outfile, "0x%.04x\n", toNum(lArg1));
            continue;
        }

        uint16_t temp_instr;
        for (int i = 0; i < num_valid_opcodes; i++) {
            if (strncmp(lOpcode, opcodes_dispach_table[i].opcode, MAX_OPCODE_LENGTH) == 0) {
                temp_instr = opcodes_dispach_table[i].func(cur_address, lOpcode, lArg1, lArg2, lArg3, lArg4);
                break;
            }
        }

        fprintf(outfile, "0x%.4x\n", temp_instr);

    } while (status != DONE);
    return EXIT_SUCCESS;
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

st_entry *search_sym(char *sym_name) {
    if (SYM_TABLE == NULL) {
        fprintf(stderr, "No symbols in the symbol table!\n");
        return NULL;
    }

    /* Standard LL traversal */
    st_entry *pST_entry = SYM_TABLE;
    do {
        if (strncmp(sym_name, pST_entry->symbol, MAX_SYMBOL_LENGTH) == 0) {
            return pST_entry;
        }

        pST_entry = pST_entry->next;
    } while (pST_entry != NULL);
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
        if (strncmp(in, opcodes_dispach_table[i].opcode, MAX_OPCODE_LENGTH) == 0) {
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

int regToNum(char *reg_name) {
    for (int i = 0; i < num_valid_reg_names; i++) {
        if (strncmp(reg_name, valid_reg_names[i].reg_name, MAX_REG_NAME_LENGTH) == 0) {
            return valid_reg_names[i].reg_val;
        }
    }
    return -1;
}

uint16_t offset_calc(uint16_t cur_addr, char* label_name, uint16_t num_bits){
    st_entry *temp = search_sym(label_name);
    uint16_t mask = 0xFFFF << num_bits;
    return ((temp->addy - cur_addr)/2) & ~(mask);
}

OPCODE_FUNC_PROTO(add) {
    OPCODE_FUNC_INIT(0x1);

    ret_val |= (regToNum(arg1) << 9);
    ret_val |= (regToNum(arg2) << 6);

    uint16_t sr2;
    if (sr2 = regToNum(arg3) != -1) {
        return ret_val | sr2;
    }
    return ret_val | (toNum(arg3) & 0x1F) | (1 << 5);
}

OPCODE_FUNC_PROTO(and) {
    OPCODE_FUNC_INIT(0x5);

    ret_val |= (regToNum(arg1) << 9);
    ret_val |= (regToNum(arg2) << 6);

    uint16_t sr2;
    if (sr2 = regToNum(arg3) != -1) {
        return ret_val | sr2;
    }
    return ret_val | (toNum(arg3) & 0x1F) | (1 << 5);
}

OPCODE_FUNC_PROTO(br) {
    OPCODE_FUNC_INIT(0x0);

    ret_val |= (0x7 << 9);

    return ret_val | offset_calc(cur_addr, arg1, 9);
}

OPCODE_FUNC_PROTO(brn) {
    return OPCODE_FUNC_NAME(br)(cur_addr, opcode, arg1, arg2, arg3, arg4) ^ (0x3 << 9);
}

OPCODE_FUNC_PROTO(brz) {
    return OPCODE_FUNC_NAME(br)(cur_addr, opcode, arg1, arg2, arg3, arg4) ^ (0x5 << 9);
}

OPCODE_FUNC_PROTO(brp) {
    return OPCODE_FUNC_NAME(br)(cur_addr, opcode, arg1, arg2, arg3, arg4) ^ (0x6 << 9);
}

OPCODE_FUNC_PROTO(brzp) {
    return OPCODE_FUNC_NAME(br)(cur_addr, opcode, arg1, arg2, arg3, arg4) ^ (0x1 << 11);
}

OPCODE_FUNC_PROTO(brnp) {
    return OPCODE_FUNC_NAME(br)(cur_addr, opcode, arg1, arg2, arg3, arg4) ^ (0x1 << 10);
}

OPCODE_FUNC_PROTO(brnz) {
    return OPCODE_FUNC_NAME(br)(cur_addr, opcode, arg1, arg2, arg3, arg4) ^ (0x1 << 9);
}

OPCODE_FUNC_PROTO(brnzp) {
    return OPCODE_FUNC_NAME(br)(cur_addr, opcode, arg1, arg2, arg3, arg4);
}

OPCODE_FUNC_PROTO(halt) {
    return 0;
}

OPCODE_FUNC_PROTO(jmp) {
    OPCODE_FUNC_INIT(0xC);
    return ret_val | (regToNum(arg1) << 6);
}

OPCODE_FUNC_PROTO(jsr) {
    OPCODE_FUNC_INIT(0x4);
    return ret_val | offset_calc(cur_addr, arg1, 11) | (1 << 11);
}

OPCODE_FUNC_PROTO(jsrr) {
    OPCODE_FUNC_INIT(0x4);
    return ret_val | (regToNum(arg1) << 6);
}

OPCODE_FUNC_PROTO(ldb) {
    OPCODE_FUNC_INIT(0x2);
    ret_val |= regToNum(arg1) << 9;
    ret_val |= regToNum(arg2) << 6;
    return ret_val | toNum(arg3);
}

OPCODE_FUNC_PROTO(ldw) {
    return 0;
}

OPCODE_FUNC_PROTO(lea) {
    return 0;
}

OPCODE_FUNC_PROTO(nop) {
    return 0;
}

OPCODE_FUNC_PROTO(not) {
    return 0;
}

OPCODE_FUNC_PROTO(ret) {
    OPCODE_FUNC_INIT(0xC);
    return ret_val | (0x7 << 6);
}

OPCODE_FUNC_PROTO(lshf) {
    return 0;
}

OPCODE_FUNC_PROTO(rshfl) {
    return 0;
}

OPCODE_FUNC_PROTO(rshfa) {
    return 0;
}

OPCODE_FUNC_PROTO(rti) {
    return 0;
}

OPCODE_FUNC_PROTO(stb) {
    return 0;
}

OPCODE_FUNC_PROTO(stw) {
    return 0;
}

OPCODE_FUNC_PROTO(trap) {
    return 0;
}

OPCODE_FUNC_PROTO(xor) {
    return 0;
}
