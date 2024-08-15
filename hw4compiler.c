// Jake Weber
// Eduardo Salcedo Fuentes
// 4/8/24
// HW4

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NORW 14
#define IMAX 5
#define CMAX 11
#define STRMAX 256
#define CODE_SIZE 1000
#define MAX_SYMBOL_TABLE_SIZE 500
#define LVLMAX 3

typedef struct {
    int kind; // const = 1, var = 2, proc = 3
    char name[CMAX]; // name up to 11 chars
    int val; // number (ASCII value)
    int level; // L level
    int addr; // M address
    int mark; // to indicate unavailable or deleted
} symbol;


typedef enum {oddsym = 1, identsym = 2, numbersym = 3, plussym = 4, minussym = 5,
multsym = 6, slashsym = 7, fisym = 8, eqlsym = 9, neqsym = 10, lessym = 11, leqsym =
12, gtrsym = 13, geqsym = 14, lparentsym = 15, rparentsym = 16, commasym = 17,
semicolonsym = 18, periodsym = 19, becomessym = 20,
beginsym = 21, endsym = 22, ifsym = 23, thensym = 24, whilesym = 25, dosym = 26, 
callsym = 27, constsym = 28, varsym = 29, procsym = 30, writesym = 31,
readsym = 32} token_type;

typedef enum { LIT = 1, OPR = 2, LOD = 3, STO = 4, CAL = 5, INC = 6, JMP = 7, JPC = 8, SYS = 9
} opCode;

char *opCodeNames[] = {"", "LIT", "OPR", "LOD", "STO", "CAL", "INC", "JMP", "JPC", "SYS"};

typedef enum { ADD = 2, SUB = 2, MUL = 2, DIV = 2, EQL = 2, NEQ = 2, LSS = 2, LEQ = 2, GTR = 2, GEQ = 2, ODD = 2
} OPR_type;

typedef enum { ONE = 1, TWO = 2, THREE = 3
} SYS_type;

typedef struct IR {
    char op[500];
    //int op;
    int L, M;
} IR;

// Global Variables
char** tokenList;
int token = 0;
symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];
int symbolIndex = 0;
IR text[500];
int cIdx = 0;
char *stringBuilder;
int level = -1;
int addr = 3;
int symbolTop = 0;
int isAssign = 0;
int numHere = 0;
//int expressFlag = 0;
int isrelop = 0;

/* list of reserved word names */
char *word[] = { "odd", "begin", "fi", "const", "do", "end", "if", "read", "then", "var", "while", "write", "call", "procedure"};

/* internal representation of reserved words */
int wsym [ ] = { oddsym, beginsym, fisym, constsym, dosym, endsym, ifsym, readsym, thensym, varsym, whilesym, writesym, callsym, procsym};

void printing(token_type *lexeme, char** iden, int lexemeMax, char** intarr, char** substrarr, int skip[]);
int isSymbol(char c);
int isLet(char c);
int isNum(char c);
int isReserved(char* string);
int convert(char* string);
char* substring(const char* input, int start, int end);

// Function Prototypes
void program();
void block();
void const_declaration();
int var_declaration();
void factor();
void emit(int op, int L, int M);
void term();
void statement();
void condition();
void expression();
void factor();
void proc_declaration();
void printEmit(FILE* fp2);
void enter(int kind, char* name, int val, int level, int address);
int find(char* ident);

int main(int argc, char** argv) {
    char words[500];
    token_type *lexeme = malloc(500 * sizeof(token_type)); // Allocate memory for lexeme
    // HAS TO RUN WITH THE EUSTIS COMMAND: “./a.out input_file.txt”
    FILE* fp = fopen("input_file.txt", "r");
    // CHANGE TO: "FILE* fp = fopen(argv[1], "r");
    //FILE* fp2 = fopen("output.txt", "w");
    FILE* fp2 = fopen("elf.txt", "w");
    char** iden = malloc(500 * sizeof(char*)); // Name array
    char** intarr = malloc(500 * sizeof(char*)); // int array
    char** substrarr = malloc(500*sizeof(char*)); // char array
    for (int i = 0; i < 500; i++)
        substrarr[i] = malloc (500 * sizeof(char));
    int index = 0; //which token
    int count = 0; //with char in string
    int idenCount = 0; 
    int intCount = 0;
    int hold;
    int start = 0;
    char* substr = (char*) malloc (100 * sizeof(char));
    int templen = 0;
    int substrind = 0;
    int skip[500];
    skip[0] = -1;
    int skipind = 0;
    stringBuilder = malloc(30 * sizeof(char));
    //strcpy(stringBuilder, "Source program:\n\n");
    char symb[500];
    while(fgets(words, 500, fp) != NULL ) // Loop through current line
    {
        //printf("%s", words);
        stringBuilder = realloc(stringBuilder, (strlen(stringBuilder) + strlen(words) + 1) * sizeof(char));
        strcat(stringBuilder, words);
        count = 0; // Index of token
        while (count != strlen(words)) // Loop through each identifier/ symbol/ operator/ reserved word/ number
        {
            start = count;
            if (isspace(words[count])) {
                count++;
                continue;
            }
            if(isLet(words[count])) // Start of reserved word/ identifier
            {
                while (isLet(words[count]) || isNum(words[count])) // Loop through input until there's a symbol or whitespace
                    count++;
                strcpy(substr, substring(words, start, count));
                if(count - start > CMAX ) // Error: Name too long
                {
                    substr = realloc(substr, strlen(substr) + 24);
                    strcat(substr, "\tError: Name too long\n");
                    strcpy(substrarr[substrind++], substr);
                    printf("Error: Name too long\n");
                    exit(0);
                    skip[skipind++] = index;
                    continue;
                }


                hold = isReserved(substr); // Check if substring is a Reserved word
                if(hold != NORW) { // Reserved word found
                    lexeme[index] = wsym[hold]; // Add (reserved word) to lexeme
                    index++;
                    strcpy(substrarr[substrind], substr);
                    substrind++;
                } else { // Identifier found
                    templen = strlen(substr);
                    lexeme[index] = identsym; // Add 2 (identsym) to lexeme
                    index++;
                    iden[idenCount] = malloc(templen + 1);
                    strcpy(iden[idenCount], substr);
                    iden[idenCount][templen] = '\0';
                    idenCount++;
                }
                continue;
            } else if(isNum(words[count])) {  // Start of number
                while (isNum(words[count]))
                    count++;
                strcpy(substr, substring(words, start, count));
                if(count - start > IMAX) { // Error: Number too long
                    substr = realloc(substr, strlen(substr) + 26);
                    strcat(substr, "\tError: Number too long\n");
                    strcpy(substrarr[substrind++], substr);
                    skip[skipind++] = index;
                    printf("Error: Number too long\n");
                    exit(0);
                    continue;
                }
                templen = strlen(substr);
                intarr[intCount] = malloc(templen + 1);
                strcpy(intarr[intCount], substr);
                intarr[intCount][templen] = '\0';
                intCount++;
                lexeme[index++] = numbersym;
            } // Check if a SYMBOL is read:
              // ‘+’, ‘-‘, ‘*’, ‘/’, ‘(‘, ‘)’, ‘=’, ’,’, ‘.’, ‘<’, ‘>’, ‘;’, ’:’ 
            else if (isSymbol(words[count])) { // Symbol found
                if (words[count]  == '+') {
                    strcpy(substrarr[substrind], "+");
                    substrind++;
                    lexeme[index++] = plussym;
                } else if (words[count]  == '-') {
                    strcpy(substrarr[substrind], "-");
                    substrind++;
                    lexeme[index++] = minussym;
                } else if (words[count]  == '*') {
                    strcpy(substrarr[substrind], "*");
                    substrind++;
                    lexeme[index++] = multsym;
                } else if(words[count] == '/') {
                    if (words[count + 1] == '*') { // Comment found
                        count += 2;
                        //printf("comment start\n");
                        while (words[count] != '*' ||  words[count + 1] != '/') {
                            count++;
                            if (words[count] == '\n') {
                                fgets(words, 500, fp);
                                stringBuilder = realloc(stringBuilder, (strlen(stringBuilder) + strlen(words) + 1) * sizeof(char));
                                strcat(stringBuilder, words);
                                count = 0;
                            }
                        }
                        count += 2;
                    } else {
                        lexeme[index++] = slashsym;
                        strcpy(substrarr[substrind], "/");
                        substrind++;
                    }
                }
                else if(words[count] == '(') {
                    strcpy(substrarr[substrind], "(");
                    substrind++;
                    lexeme[index++] = lparentsym;
                } else if(words[count] == ')') {
                    strcpy(substrarr[substrind], ")");
                    substrind++;
                    lexeme[index++] = rparentsym;
                } else if(words[count] == '=') {
                    strcpy(substrarr[substrind], "=");
                    substrind++;
                    lexeme[index++] = eqlsym;
                } else if(words[count] == ',') {
                    strcpy(substrarr[substrind], ",");
                    substrind++;
                    lexeme[index++] = commasym;
                } else if(words[count] == '.') {
                    strcpy(substrarr[substrind], ".");
                    substrind++;
                    lexeme[index++] = periodsym;
                } else if(words[count] == '<') {
                    if (words[count + 1] == '=') {
                        strcpy(substrarr[substrind], "<=");
                        substrind++;
                        lexeme[index++] = leqsym;
                        count++;
                    }else if (words[count + 1] == '>') {
                        strcpy(substrarr[substrind], "<>");
                        substrind++;
                        lexeme[index++] = neqsym;
                        count++;
                    } else {
                        strcpy(substrarr[substrind], "<");
                        substrind++;
                        lexeme[index++] = lessym;
                    }
                } else if(words[count] == '>') {
                    if (words[count + 1] == '=') {
                        strcpy(substrarr[substrind], ">=");
                        substrind++;
                        lexeme[index++] = geqsym;
                        count++;
                    } else {
                        strcpy(substrarr[substrind], ">");
                        substrind++;
                        lexeme[index++] = gtrsym;
                     }
                } else if(words[count] == ';') {
                    strcpy(substrarr[substrind], ";");
                    substrind++;
                    lexeme[index++] = semicolonsym;
                } else if(words[count] == ':') {
                    if (words[count + 1] == '=') {
                        strcpy(substrarr[substrind], ":=");
                        substrind++;
                        lexeme[index++] = becomessym;
                        count++;
                    } // Case for " : ", display invalid symbols error message
                    else {
                        //printf("test\n");
                        for (int i = 0; i < strlen(words); i++)
                            substr[i] = '\0';
                        substr = realloc(substr, strlen(substr) + 25);
                        substr[0] = ':';
                        substr = strcat(substr, "\t\tError: Invalid symbol\n");
                        strcpy(substrarr[substrind++], substr);
                        printf("Error: Invalid symbol\n");
                        exit(0);
                        skip[skipind++] = index;
                    }
                 } 
                count++;
                continue;
            } else {  // Error: Invalid symbols
                for (int i = 0; i < strlen(words); i++)
                    substr[i] = '\0';
                substr = realloc(substr, strlen(substr) + 25);
                substr[0] = words[count++];
                substr = strcat(substr, "\t\tError: Invalid symbol\n");
                strcpy(substrarr[substrind++], substr);
                skip[skipind++] = index;
                printf("Error: Invalid symbol\n");
                exit(0);
             }
        }
    }
    idenCount = 0;
    intCount = 0;
    skipind = 0;
    tokenList = malloc(500 * sizeof(char*));
    for (int i = 0; i < 500; i++)
        tokenList[i] = malloc(500 * sizeof(char));

    // REAL PRINTING CODE:
    //printing(lexeme, iden, index, intarr, substrarr, skip);
    // NO NEED TO PRINT LEXEME TABLE
    //printing(lexeme, iden, index, intarr, substrarr, skip);


    // NO LONGER NEED TO PRINT TOKEN LIST
    //printf("\nToken List:\n");
    int tokenListCount = 0;
    for(int i = 0; i < index ; i++) {
        sprintf(tokenList[tokenListCount], "%d", lexeme[i]);
        //printf("%s ", tokenList[tokenListCount]);
        tokenListCount++;
        if(lexeme[i] == 2) {
            sprintf(tokenList[tokenListCount], "%s", iden[idenCount++]);
            //printf("%s ", tokenList[tokenListCount]);
            tokenListCount++;
        } else if(lexeme[i] == 3) {
            sprintf(tokenList[tokenListCount], "%s", intarr[intCount++]);
            //printf("%s ", tokenList[tokenListCount]);
            tokenListCount++;
        }
    }
    
    //Reads and evaluates syntax and checks for eroor
    program ();
    //print code and symbol table
    printf("%s", stringBuilder);
    printf("\n\nNo errors, program is syntactically correct\n");
    printf("\nAssembly Code:\n\n"); 
    printEmit(fp2);


    // NO NEED TO PRINT SYMBOL TABLE
    printf("\nSymbol Table: (FOR TESTING, DELETE LATER)\n\n");
    printf("Kind | Name\t\t\t| Value | Level | Address | Mark\n");
    printf("-------------------------------------------------\n");
    for (int i = 0; i < symbolTop; i++) {
        printf("   %d |%13s", symbol_table[i].kind, symbol_table[i].name);
        printf(" |\t  %d |\t  %d", symbol_table[i].val, symbol_table[i].level);
        printf(" |\t\t%d |\t  %d\n", symbol_table[i].addr, symbol_table[i].mark);
    }

    //printf("%d", symbolIndex);

    // Free all elements
    for (int i = 0; i < 500; i++)
        free(substrarr[i]);
    free(substrarr);
    free(substr);
    for (int i = 0; i < 500; i++)
        free(tokenList[i]);
    free(tokenList);
    for (int i = 0; i < idenCount; i++)
        free(iden[i]);
    free(iden);
    for (int i = 0; i < intCount; i++)
        free(intarr[i]);
    free(intarr);
    free(lexeme);

    fclose(fp);
    fclose(fp2);

    return 0;

}

void printing(token_type *lexeme, char** iden, int lexemeMax, char** intarr, char** substrarr, int skip[]) {
    printf("\n\nLexeme Table:\n");
    printf("\nlexeme\ttoken type\n");
    int track1 = 0;
    int track2 = 0;
    int track3 = 0;
    int skipind = 0;
    for (int i = 0; i < lexemeMax; i++) { // Loop through every lexeme, print from substrarr (substring array) too
        if(i == skip[skipind]){ // Print error message
            printf("%s", substrarr[track3++]);
            skip[skipind++] = -1;
            i--;
        }
        else if (lexeme[i] == 2) { // Print identifier
            printf("%s", iden[track1]);
            track1++;
            printf("\t\t2\n");
        } else if (lexeme[i] == 3) { // Print number
            printf("%s", intarr[track2]);
            track2++;
            printf("\t\t3\n");
        } else { // Print all other cases (not error, identifier, or number)
           printf("%s", substrarr[track3]);
            track3++;
            if (strlen(substrarr[track3 - 1]) > 5) printf("\t%d\n", lexeme[i]);
            else printf("\t\t%d\n", lexeme[i]);
        }
    }

}

//checks if is a symbol
int isSymbol(char c) {
    //‘+’, ‘-‘, ‘*’, ‘/’, ‘(‘, ‘)’, ‘=’, ’,’ , ‘.’, ‘ <’, ‘>’, ‘;’ , ’:’ 
    if (c == '+'  || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || c == '=' || c == ',' || c == '.' || c == '<' || c == '>' || c == ';' || c == ':') return 1;
    else return 0;
}

//Searches for symbol in symbol table
int symbolTableCheck (char* identifier) {
    for (int i = 0; i < 500; i++) {
        if (strcmp(identifier, symbol_table[i].name) == 0 && symbol_table[i].level == level)
            return i;
    }
    return -1;
}

//Letter check
int isLet(char c) {
  if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) 
    return 1;
  else 
    return 0; 
}

//Number check
int isNum(char c) {
  if (c >= '0' && c <= '9')
      return 1;
    else 
      return 0; 
}

//check if a word is reserved
int isReserved(char* string){
  for(int i = 0; i < NORW; i++)
    if(strcmp(string, word[i]) == 0){
      return i;
    }
  return NORW;
}

//convert a string to a int
int convert (char* string) {
  return atoi(string);
}

//take a sub string from start to end of a string
char* substring(const char* input, int start, int end) {
    int length = end - start;
    char* sub = (char*)malloc((length + 1) * sizeof(char)); // Allocate memory for the substring (+1 for the null terminator)
    strncpy(sub, input + start, length);
    sub[length] = '\0'; // Null terminate the substring
    return sub;
}


//Program code
void program () {
    block();
    //check for period at end
    if (atoi(tokenList[token]) != periodsym) {
        printf("Error: program must end with period\n");
        exit(0);
    }
    //emit sys end commands
    emit(SYS, 0, 3);
    //mark all symbols in main
    for (int i = 0; i < symbolIndex; i++) {
        if (symbol_table[i].level == 0)
            symbol_table[i].mark = 1;
    }
}

//block function
void block () {
    level++;
    addr = 3;
    int cIdx2, start = 3, holdIdx = cIdx, holdSym = symbolIndex;
    emit(JMP, 0, 0);
    //start with jmp as customary
    //emit(JMP, 0, 3); IDK YET
    if(level > LVLMAX){
        printf("Error: Surpassed level limit\n"); //too many procedure calls
        exit(0);
    }
    //declare constants
    const_declaration();
    if (atoi(tokenList[token]) == constsym) {
        printf("Error: can't declare const twice\n");
        exit(0);
    }
    //declare vars 
    int numVars = var_declaration();
    if (atoi(tokenList[token]) == varsym || atoi(tokenList[token]) == constsym) {
        printf("Error: can't declare var/ const twice\n");
        exit(0);
    }
    //declare procedures
    proc_declaration();
    if (atoi(tokenList[token]) == constsym || atoi(tokenList[token]) == varsym) {
        printf("Error: can't declare var/ const twice\n");
        exit(0);
    }
    //mark all symbols in the completed procedure
    for (int i = 0; i < symbolIndex; i++) {
        if (symbol_table[i].level == level)
            symbol_table[i].mark = 1;
    }
    text[holdIdx].M = 3 * cIdx;
    //symbol_table[holdIdx].addr = cIdx;
    cIdx2 = cIdx;
    //increment based on number of vars declared and initial jmp
    emit(INC, 0, start + numVars);
    //statements begin
    statement();
    if(level != 0)
        emit(OPR, 0, 0);
    symbolIndex = holdSym;
    level--;
}

void const_declaration () {
    if (atoi(tokenList[token]) == becomessym) {
        printf("Error: var must precede becomessym");
        exit(0);
    }
    //check for constantsym
    if (atoi(tokenList[token]) == constsym) {
        do {
            // get next token;
            token++;
            //check for identifier next
            if (atoi(tokenList[token]) != identsym) {
                //error with no identifier
                printf("Error: const, var, and read keywords must be followed by identifier\n");
                exit(0);
            }
            //get next token check constant's name
            token++;
            if (symbolTableCheck(tokenList[token]) != -1) {
                //error if name was already declared
                printf("Error: symbol name has already been declared\n");
                exit(0);
            }
            //save identifier name
            char *ident = malloc(12 * sizeof(char));
            ident = tokenList[token];

            // get next token
            token++;

            //error if constant is not assigned/followed by equal sign
            if (atoi(tokenList[token]) != eqlsym) {
                printf("Error: constants must be assigned with =\n");
                exit(0);
            }
            // get next token, looking for number now
            token++;
            //error if there's no number after the equals
            if (atoi(tokenList[token]) != numbersym) {
                printf("Error: constants must be assigned an integer value\n");
                exit(0);
            }
            //get next token, which is the number value
            token++;
            //add constant to symbol table (kind 1, saved name, value, 0, 0, 0)
            enter(1, ident, atoi(tokenList[token]), level, 0);

            // get next token
            token++;
        }
        while (atoi(tokenList[token]) == commasym); //repeat while a comma follows the const declaration
        //once there are no commas, the declaration should end in a semicolon
        if (atoi(tokenList[token]) != semicolonsym) {
            printf("Error: constant and variable declarations must be followed by a semicolon\n");
            exit(0);
        }
        // get next token
        token++;
    }
}

//variable declaration
int var_declaration () {
    if (atoi(tokenList[token]) == becomessym) {
        printf("Error: var must precede becomessym");
        exit(0);
    }
    int numVars = 0; //counts the number of variables declared for the return val
    //check for varsym to start declaration
    if (atoi(tokenList[token]) == varsym) {
        do {
            //varsym found, count one var
            numVars++;
            // get next token
            token++;
            //check for identifier next
            if (atoi(tokenList[token]) != identsym) {
                //error with no identifier
                printf("Error: const, var, and read keywords must be followed by identifier");
                exit(0);
            }
            //increment to get identifier name
            token++;
            int curSymbolIndex = symbolTableCheck(tokenList[token]);
            //error if already declared
            if (curSymbolIndex != -1 ) {
                printf("Error: symbol name has already been declared\n\n");
                exit(0);
            }
            //add to symbol table (kind 2, ident, 0, var# + 2, 0, 0)
            enter(2, tokenList[token], 0, level, numVars + 2);

            // get next token;
            token++;
        }
        while (atoi(tokenList[token]) == commasym); //repeat while a comma follows the var declaration
        //once there are no commas, the declaration should end in a semicolon
        if (atoi(tokenList[token]) != semicolonsym) {
            printf("Error: constant and variable declarations must be followed by a semicolon");
            exit(0);
        }
        // get next token
        token++;
    }
    //returns number of variables to prepare to increment space
    return numVars;
}

void proc_declaration() {
    if (atoi(tokenList[token]) == becomessym) {
        printf("Error: var must precede becomessym");
        exit(0);
    }
    while (atoi(tokenList[token]) == procsym) {
        token++;
        if (atoi(tokenList[token]) != identsym) {
            printf("Error: Identifier needed after procedure declaration\n");
            exit(0);
        }
        //increment to get identifier name
        token++;
        //error if already declared
        if (symbolTableCheck(tokenList[token]) != -1) {
            printf("Error: symbol name has already been declared\n\n");
            exit(0);
        }
        //add to symbol table (kind 3, ident, 0, ?, ?, ?) <---------------IDK YET
        enter(3, tokenList[token], 0, level, 3 * cIdx);
        token++;
        if (atoi(tokenList[token]) != semicolonsym) {
            printf("Semicolon between statements missing\n");
            exit(0);
        }
        token++;
        block();
        if (atoi(tokenList[token]) != semicolonsym) {
            printf("Semicolon or comma missing\n");
            exit(0);
        }
        token++;
    }
}

void statement() {
    if (atoi(tokenList[token]) == becomessym) {
        printf("Error: var must precede becomessym");
        exit(0);
    }
    if (atoi(tokenList[token]) == constsym || atoi(tokenList[token]) == varsym) {
        printf("Error: statement expected\n");
        exit(0);
    }
    if (atoi(tokenList[token]) == identsym) { //if identifer, get next token and check symbol table
        token++;
        int curSymbolIndex = find(tokenList[token]);
        //error if name is undeclared
        if (curSymbolIndex == -1) {
            printf("Error: undeclared identifier %s\n", tokenList[token]);
            exit(0);
        }
        //error if identifier is of a constant (or anything other than var)
        if (symbol_table[curSymbolIndex].kind != 2){
            printf("Error: only variable values may be altered\n");
            exit(0);
        }
        
        // get next token
        token++;
        //error if identifier is not assigned value with ":="
        if (atoi(tokenList[token]) != becomessym) {
            printf("Error: assignment statements must use :=\n");
            exit(0);
        }
        // get next token
        token++;

        expression(); //at the end of the statement there is an expression, and the res value is stored
        emit(STO, level - symbol_table[curSymbolIndex].level, symbol_table[curSymbolIndex].addr);
        //symbol_table[curSymbolIndex].mark = 1;
        return;
    } 
    if (atoi(tokenList[token]) == callsym) { //Procedure call
        token++;
        if (atoi(tokenList[token]) != identsym) { 
            printf("Error: call must be followed by an identifier\n");
            exit(0);
        } //if identifer, get next token and check symbol table
        token++;
        int curSymbolIndex = find(tokenList[token]);
        //error if name is undeclared
        if (curSymbolIndex == -1) {
            printf("Error: undeclared identifier\n");
            exit(0);
        } 
        //error if identifier is of anything other than procedure
        if (symbol_table[curSymbolIndex].kind != 3) {
            printf("Error: Call of a constant or variable is meaningless\n");
            exit(0);
        }
        emit(CAL, level - symbol_table[curSymbolIndex].level, symbol_table[curSymbolIndex].addr);
        token++;
        return;
    } 
    if (atoi(tokenList[token]) == beginsym) { //if begin, another statement occurs
        do {
            // get next token
            token++;

            statement();

        } while (atoi(tokenList[token]) == semicolonsym); //while statements happen (End in semicolon)
        //error if begin does not have a matching "end"
        if (atoi(tokenList[token]) != endsym) {
            printf("Error: begin must be followed by end\n");
            exit(0);
        }
        // get next token
        token++;
        return;
    } 
    if (atoi(tokenList[token]) == ifsym) { //if if found
        // get next token
        token++;
        //check the condition
        condition();
        //set the jump index to current index
        int jpcIdx = cIdx;
        emit(JPC, 0, jpcIdx);
        //error if then does not follow if
        if (atoi(tokenList[token]) != thensym) {
            printf("Error: if must be followed by then\n");
            exit(0);
        }
        // get next token
        token++;
        statement(); //statement within if
        //error if if does not end with fi
        if (atoi(tokenList[token]) != fisym) {
            printf("Error: if must end with a fi\n");
            exit(0);
        }
        token++;
        //set appropriate index to jump to after recursive calls
        text[jpcIdx].M = 3 * cIdx;
        return;
    } 
    if (atoi(tokenList[token]) == whilesym){ //while encountered
        // get next token
        token++;
        //jump index set to current index
        int loopIdx = cIdx;
        condition(); //condition for while
        if (atoi(tokenList[token]) != dosym) { //error if while does not have do
            printf("Error: while must be followed by do\n");
            exit(0);
        }
        // get next token
        token++;
        int jpcIdx = cIdx; //set jump condition index to current index
        emit(JPC, 0, jpcIdx);
        statement();
        emit(JMP, 0, 3 * loopIdx);
        //set appropriate index to jump to after recursive calls
        text[jpcIdx].M = 3 * cIdx;
        return;
    } 
    if (atoi(tokenList[token]) == readsym) { //read
        // get next token
        token++;
        if (atoi(tokenList[token]) != identsym) { //error if identier doesn't follow read
            printf("Error: const, var, and read keywords must be followed by identifier\n");
            exit(0);
        }
        token++; //next token should be the name 
        int curSymbolIndex = find(tokenList[token]);
        //error if identifier undeclared
        if (curSymbolIndex == -1) {
            printf("Error: undeclared identifier\n");
            exit(0);
        }
        //error if not variable
        if (symbol_table[curSymbolIndex].kind != 2) {
            printf("Error: only variable values may be altered\n");
            exit(0);
        }
        // get next token
        token++;
        emit(SYS, 0, 2);
        emit (STO, level - symbol_table[curSymbolIndex].level, symbol_table[curSymbolIndex].addr);
        return;
    } 
    if (atoi(tokenList[token]) == writesym) { //write
        // get next token
        token++;

        expression();
        emit(SYS, 0, 1);
        return;
    }
}  

void condition() {
    //checks which OPR (operation) is being done
    isAssign = 0;
    numHere = 0;
    isrelop = 0;
    if (atoi(tokenList[token]) == oddsym) {
        isrelop = 1;
        // get next token
        token++;
        expression();
        emit(ODD, 0, 11);
    } else {
        expression();
        if (atoi(tokenList[token]) == eqlsym) { //equals
            isrelop = 1;
            // get next token
            token++;
            expression();
            emit(EQL, 0, 5);
        } else if (atoi(tokenList[token]) == neqsym) { //not equals
            isrelop = 1;
            // get next token
            token++;
            expression();
            emit(NEQ, 0, 6);
        } else if (atoi(tokenList[token]) == lessym) { //less than
            isrelop = 1;
            // get next token
            token++;
            expression();
            emit(LSS, 0, 7);
        } else if (atoi(tokenList[token]) == leqsym) { //less than or equal
            isrelop = 1;
            // get next token
            token++;
            expression();
            emit(LEQ, 0, 8);
        } else if (atoi(tokenList[token]) == gtrsym) { //greater than
            isrelop = 1;
            // get next token
            token++;
            expression();
            emit(GTR, 0, 9);
        } else if (atoi(tokenList[token]) == geqsym) { //greater than or equal
            isrelop = 1;
            // get next token
            token++;
            expression();
            emit(GEQ, 0, 10);
        } else { //error if condition has no operator
            if (isAssign) {
                printf("Error: Use = instead of :=\n");
                exit(0);
            }
            printf("Error: condition must contain comparison operator\n");
            exit(0);
        }
    }
    isAssign = 0;
    numHere = 0;
    isrelop = 0;
}

void expression() {
    //expressFlag = 0;
    term(); //expression starts with a term
    while (atoi(tokenList[token]) == plussym || atoi(tokenList[token]) == minussym) { //looks for + or -
        if (atoi(tokenList[token]) == plussym) { //plus
            // get next token
            token++;
            term(); //term following plus
            emit(ADD, 0, 1);
        } else if (atoi(tokenList[token]) == minussym) { //minus
            // get next token
            token++;
            term(); //term following minus
            emit(SUB, 0, 2);
        }
    }
}

void term() {
    factor(); //term starts with factor
    while (atoi(tokenList[token]) == multsym || atoi(tokenList[token]) == slashsym) { //look for * or /
        if (atoi(tokenList[token]) == multsym) { //multiplication
            // get next token
            token++;
            factor(); //factor following *
            emit(MUL, 0, 3);
        }
        else if (atoi(tokenList[token]) == slashsym) { //division
            // get next token
            token++;
            factor(); //factor following slash
            emit(DIV, 0, 4);
        }
    }
}

void factor() {
    //check for identifier
    if(atoi(tokenList[token]) == identsym) {
        token++;
        int curSymbolIndex = find(tokenList[token]);
        if (curSymbolIndex == -1) { //error if not declared
            printf("Error: undeclared identifier\n");
            exit(0);
        }
        if (symbol_table[curSymbolIndex].kind == 3) {
            printf("Error: Expression must not contain a procedure identifier\n");
            exit(0);
        }
        if (symbol_table[curSymbolIndex].kind == 1) { // constant leads to LIT
            emit(LIT, 0, symbol_table[curSymbolIndex].val);
            //symbol_table[curSymbolIndex].mark = 1;
        } else { // variable leads to LOD
            emit(LOD, level - symbol_table[curSymbolIndex].level, symbol_table[curSymbolIndex].addr);
            //symbol_table[curSymbolIndex].mark = 1;
        }
        // get next token
        token++;
        numHere= 1;
        if (atoi(tokenList[token]) == becomessym)
            isAssign = 1;
    } else if (atoi(tokenList[token]) == numbersym) { //otherwise check for number
        token++;
        emit(LIT, 0, atoi(tokenList[token]));
        // get next token
        token++;
        numHere = 1;
        return;
    } else if (atoi(tokenList[token]) == lparentsym) { //left parentheses
        // get next token
        token++;
        expression();
        if (atoi(tokenList[token]) != rparentsym) { //left parentheses needs to have end parentheses after expression
            printf("Error: right parenthesis must follow left parenthesis\n");
            exit(0);
        }
        // get next token
        token++;
    } else { //error if nothing is found
        if (atoi(tokenList[token - 1]) == becomessym || atoi(tokenList[token - 1]) == lparentsym) {
            printf("Error: An expression cannot begin with this symbol\n");
            exit(0);
        }
        if (isrelop == 1 || atoi(tokenList[token - 1]) == ifsym || atoi(tokenList[token - 1]) == whilesym) {
            printf("Error: An expression cannot begin with this symbol\n");
            exit(0);
        }
        if (numHere) {
            printf("Error: = must be followed by a number\n");
            exit(0);
        }
        printf("Error: arithmetic equations must contain operands, parentheses, numbers, or symbols\n");
        exit(0);
    }
}

    //copy IRs
void emit(int op, int L, int M) {
    strcpy(text[cIdx].op, opCodeNames[op]); // opcode
    text[cIdx].L = L; // lexicographical level
    text[cIdx].M = M; // modifier
    cIdx++;
}

    //print emit table
void printEmit(FILE* fp2) {
    printf("Line\tOP\t\tL\t\tM\n");
    for(int i = 0; i < cIdx; i++) {
        if (strlen(text[i].op) >= 4)
            printf(" %d\t\t%s\t%d\t\t%d\n", i , text[i].op, text[i].L, text[i].M );
        else
            printf(" %d\t\t%s\t\t%d\t\t%d\n", i , text[i].op, text[i].L, text[i].M );

        int tempOp;
        if (strcmp(text[i].op, "LIT") == 0) tempOp = 1;
        else if (strcmp(text[i].op, "OPR") == 0) tempOp = 2;
        else if (strcmp(text[i].op, "LOD") == 0) tempOp = 3;
        else if (strcmp(text[i].op, "STO") == 0) tempOp = 4;
        else if (strcmp(text[i].op, "CAL") == 0) tempOp = 5;
        else if (strcmp(text[i].op, "INC") == 0) tempOp = 6;
        else if (strcmp(text[i].op, "JMP") == 0) tempOp = 7;
        else if (strcmp(text[i].op, "JPC") == 0) tempOp = 8;
        else if (strcmp(text[i].op, "SYS") == 0) tempOp = 9;
        fprintf(fp2, "%d %d %d", tempOp, text[i].L, text[i].M);
        if (i != cIdx - 1) fprintf(fp2, "\n");
    }
}

void enter(int kind, char* name, int val, int level, int address){
    symbol_table[symbolIndex].kind = kind;
    strcpy(symbol_table[symbolIndex].name, name);
    symbol_table[symbolIndex].val = val;
    symbol_table[symbolIndex].level = level;
    symbol_table[symbolIndex].addr = address;
    symbol_table[symbolIndex].mark = 0;
    symbolIndex++;
    symbolTop++;
    addr++;
}

int find(char* ident){
    for(int i = symbolIndex - 1; i >= 0; i--){
        if(strcmp(symbol_table[i].name,ident)==0){
            return i;
        }
    }
    return -1;
}
