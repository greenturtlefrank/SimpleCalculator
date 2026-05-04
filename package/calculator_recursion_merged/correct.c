#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// for lex
#define MAXLEN 256

// Token types
typedef enum
{
    UNKNOWN, END, ENDFILE,
    INT, ID,
    ADDSUB, MULDIV,
    ASSIGN,
    LPAREN, RPAREN
    ,
    INCDEC,AND,OR,XOR


} TokenSet;

TokenSet getToken(void);
TokenSet curToken = UNKNOWN;
char lexeme[MAXLEN];

// Test if a token matches the current token
int match(TokenSet token);
// Get the next token
void advance(void);
// Get the lexeme of the current token
char *getLexeme(void);


// for parser
#define TBLSIZE 64
// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 1

// Call this macro to print error message and exit the program
// This will also print where you called it in your program
#define error(errorNum) { \
    err(errorNum); \
}

/*if (PRINTERR) \
        fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \*/

// Error types
typedef enum
{
    UNDEFINED, MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, NOTLVAL, DIVZERO, SYNTAXERR
} ErrorType;

// Structure of the symbol table
typedef struct
{
    int val;
    char name[MAXLEN];
} Symbol;

// Structure of a tree node
typedef struct _Node
{
    TokenSet data;
    int val;
    //int isid;
    char lexeme[MAXLEN];
    struct _Node *left;
    struct _Node *right;
} BTNode;

int sbcount = 0;
Symbol table[TBLSIZE];

// Initialize the symbol table with builtin variables
void initTable(void);
// Get the value of a variable
int getval(char *str);
// Set the value of a variable
int setval(char *str, int val);
// Make a new node according to token type and lexeme
BTNode *makeNode(TokenSet tok, const char *lexe);
// Free the syntax tree
void freeTree(BTNode *root);
BTNode *factor(void);

BTNode *assign_expr(void);
BTNode *or_expr(void);
BTNode *or_expr_tail(BTNode* left);
BTNode *xor_expr(void);
BTNode *xor_expr_tail(BTNode* left);
BTNode *and_expr(void);
BTNode *and_expr_tail(BTNode* left);
BTNode *addsub_expr(void);
BTNode *addsub_expr_tail(BTNode* left);
BTNode *muldiv_expr(void);
BTNode *muldiv_expr_tail(BTNode* left);
BTNode *unary_expr(void);

int getplace(char *str);
char *whatsign(int in);
int pass(BTNode* now);
int haveincdec(BTNode* now, char * in);


/*
BTNode *term(void);
BTNode *term_tail(BTNode *left);
BTNode *expr(void);
BTNode *expr_tail(BTNode *left);
*/




void statement(void);
// Print error message and exit the program
void err(ErrorType errorNum);


// for codeGen
// Evaluate the syntax tree
int evaluateTree(BTNode *root);
// Print the syntax tree in prefix
void printPrefix(BTNode *root);


/*============================================================================================
lex implementation
============================================================================================*/

TokenSet getToken(void) //READ WORD
{
    int i = 0;
    char c = '\0';

    while ((c = fgetc(stdin)) == ' ' || c == '\t');  //READ TRASH

    if (isdigit(c))
    {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN)
        {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return INT;
    }
    else if (c == '+')
    {
        lexeme[0] = c;

        c = fgetc(stdin);
        if(c == '+')
        {
            lexeme[1] = c;
            lexeme[2] = '\0';

            //printf("INCDEC\n");

            return INCDEC;
        }
        else
        {
            ungetc(c, stdin);
            lexeme[1] = '\0';

            return ADDSUB;
        }


    }
    else if (c == '-')
    {
        lexeme[0] = c;

        c = fgetc(stdin);
        if(c == '-')
        {
            lexeme[1] = c;
            lexeme[2] = '\0';

            //printf("INCDEC\n");

            return INCDEC;
        }
        else
        {
            ungetc(c, stdin);
            lexeme[1] = '\0';

            return ADDSUB;
        }


    }
    else if (c == '*' || c == '/')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    }
    else if (c == '\n')
    {
        lexeme[0] = '\0';
        return END;
    }
    else if (c == '=')
    {
        strcpy(lexeme, "=");
        return ASSIGN;
    }
    else if (c == '(')
    {
        strcpy(lexeme, "(");
        return LPAREN;
    }
    else if (c == ')')
    {
        strcpy(lexeme, ")");
        return RPAREN;
    }
    else if (isalpha(c) || c == '_')
    {
        lexeme[0] = c;


        c = fgetc(stdin);
        i = 1;
        while ( ( (isalpha(c) || c == '_' ) || isdigit(c)) && i < MAXLEN)
        {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';

        //printf("%s\n",lexeme);

        //lexeme[1] = '\0';
        return ID;
    }
    else if (c == '&')
    {
        lexeme[0] = '&';
        lexeme[1] = '\0';
        return AND;
    }
    else if (c == '|')
    {
        lexeme[0] = '|';
        lexeme[1] = '\0';
        return OR;
    }
    else if (c == '^')
    {
        lexeme[0] = '^';
        lexeme[1] = '\0';
        return XOR;



    }
    else if (c == EOF)
    {
        return ENDFILE;



    }
    else
    {
        return UNKNOWN;
    }
}

void advance(void)
{
    curToken = getToken();
}

int match(TokenSet token)       //==
{
    if (curToken == UNKNOWN)
        advance();
    return token == curToken;
}

char *getLexeme(void)
{
    return lexeme;
}


/*============================================================================================
parser implementation
============================================================================================*/

void initTable(void)
{
    strcpy(table[0].name, "x");
    table[0].val = 0;
    strcpy(table[1].name, "y");
    table[1].val = 0;
    strcpy(table[2].name, "z");
    table[2].val = 0;
    sbcount = 3;
}

int getval(char *str)
{
    int i = 0;

    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i].val;

    if (sbcount >= TBLSIZE)
        error(RUNOUT);

    error(NOTFOUND);

    /*strcpy(table[sbcount].name, str);
    table[sbcount].val = 0;
    sbcount++;*/
    return 0;
}
/*
int getvalnot(char *str) {
    int i = 0;

    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i].val;


    error(RUNOUT);


    return 0;
}
*/

int setval(char *str, int val)
{
    int i = 0;

    for (i = 0; i < sbcount; i++)
    {
        if (strcmp(str, table[i].name) == 0)
        {
            table[i].val = val;
            return val;
        }
    }

    if (sbcount >= TBLSIZE)
        error(RUNOUT);

    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;
    sbcount++;
    return val;
}

BTNode *makeNode(TokenSet tok, const char *lexe)
{
    BTNode *node = (BTNode*)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    //node->isid = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void freeTree(BTNode *root)
{
    if (root != NULL)
    {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}
















void statement(void)
{
    BTNode *retp = NULL;

    if (match(ENDFILE))
    {
        printf("MOV r0 [0]\n");
        printf("MOV r1 [4]\n");
        printf("MOV r2 [8]\n");
        printf("EXIT 0\n");
        exit(0);
    }
    else if (match(END))
    {
        //printf(">> ");
        advance();
    }
    else
    {
        retp = assign_expr();
        if (match(END))
        {
            evaluateTree(retp);
            //printf("%d\n", evaluateTree(retp));
            //printf("Prefix traversal: ");
            //printPrefix(retp);
            //printf("\n");
            freeTree(retp);
            //printf(">> ");
            advance();
        }
        else
        {
            error(SYNTAXERR);
        }
    }
}



BTNode *assign_expr(void)
{
    //int in = 0;
    BTNode* back, *left;

    int idflag = match(ID);
    left = or_expr();



    if(match(ASSIGN))
    {
        if(idflag)
        {
            back = makeNode(ASSIGN, getLexeme() );
            back -> left = left;
            advance();
            back -> right = assign_expr();
        }
        else
        {
            error(NOTNUMID);
        }

    }
    else
    {
        back = left;
    }





    return back;
}



BTNode *or_expr(void)
{
    BTNode* back;


    back = xor_expr();



    return or_expr_tail(back);
}

BTNode *or_expr_tail(BTNode* left)
{
    BTNode* back;

    if(match(OR))
    {
        back = makeNode(OR,getLexeme());
        advance();
        back -> left = left;
        back -> right = xor_expr();

        return or_expr_tail(back);
    }
    else
    {
        back = left;

        return back;
    }


}

BTNode *xor_expr(void)
{
    BTNode* back;


    back = and_expr();



    return xor_expr_tail(back);
}

BTNode *xor_expr_tail(BTNode* left)
{
    BTNode* back;

    if(match(XOR))
    {
        back = makeNode(XOR,getLexeme());
        advance();
        back -> left = left;
        back -> right = and_expr();

        return xor_expr_tail(back);
    }
    else
    {
        back = left;

        return back;
    }


}

BTNode *and_expr(void)
{
    BTNode* back;


    back = addsub_expr();



    return and_expr_tail(back);
}

BTNode *and_expr_tail(BTNode* left)
{
    BTNode* back;

    if(match(AND))
    {
        back = makeNode(AND,getLexeme());
        advance();
        back -> left = left;
        back -> right = addsub_expr();

        return and_expr_tail(back);
    }
    else
    {
        back = left;

        return back;
    }


}

BTNode *addsub_expr(void)
{
    BTNode* back;


    back = muldiv_expr();



    return addsub_expr_tail(back);
}

BTNode *addsub_expr_tail(BTNode* left)
{
    BTNode* back;

    if(match(ADDSUB))
    {
        back = makeNode(ADDSUB,getLexeme());
        advance();
        back -> left = left;
        back -> right = muldiv_expr();

        return addsub_expr_tail(back);
    }
    else
    {
        back = left;

        return back;
    }


}

BTNode *muldiv_expr(void)
{
    BTNode* back;


    back = unary_expr();



    return muldiv_expr_tail(back);
}

BTNode *muldiv_expr_tail(BTNode* left)
{
    BTNode* back;

    if(match(MULDIV))
    {
        back = makeNode(MULDIV,getLexeme());
        advance();
        back -> left = left;
        back -> right = unary_expr();

        return muldiv_expr_tail(back);
    }
    else
    {
        back = left;

        return back;
    }


}

BTNode *unary_expr(void)
{
    BTNode* back;

    if(match(ADDSUB))
    {
        back = makeNode(ADDSUB,getLexeme());
        advance();
        back -> left = makeNode(INT, "0\0");
        back -> right = unary_expr();

        return back;
    }
    else
    {
        back = factor();

        return back;
    }




}




// factor := INT | ADDSUB INT |
//		   	 ID  | ADDSUB ID  |
//		   	 ID ASSIGN expr |
//		   	 LPAREN expr RPAREN |
//		   	 ADDSUB LPAREN expr RPAREN
BTNode *factor(void)
{
    BTNode *retp = NULL, *left = NULL;

    if (match(INT))
    {
        retp = makeNode(INT, getLexeme());
        advance();
    }
    else if (match(ID))
    {
        retp = makeNode(ID, getLexeme());
        advance();



        /*
        if (!match(ASSIGN)) {
            retp = left;
        } else {
            retp = makeNode(ASSIGN, getLexeme());
            advance();
            retp->left = left;
            retp->right = assign_expr();
        }*/






    }
    else if (match(INCDEC))
    {

        char lex[MAXLEN];
        strcpy(lex, getLexeme());

        char* addsub = malloc(sizeof(char) * 2);
        addsub[0] = lex[0];
        addsub[1] = '\0';



        //printf("hi%c%chi\n",addsub[0],addsub[1]);

        retp = makeNode(ASSIGN, "=\0");
        advance();
        if(match(ID))
        {
            BTNode* temp;

            retp -> left = makeNode(ID, getLexeme());
            retp -> right = makeNode(ADDSUB, addsub);

            temp = retp -> right;
            temp -> left = makeNode(ID, getLexeme());
            temp -> right = makeNode(INT, "1\0");

            advance();
            //printf("kk\n");
        }
        else
        {

        }





    }
    else if (match(LPAREN))
    {
        advance();
        retp = assign_expr();
        if (match(RPAREN))
            advance();
        else
            error(MISPAREN);
    }





    /*
         else if (match(ADDSUB)) {
            retp = makeNode(ADDSUB, getLexeme());
            retp->left = makeNode(INT, "0");
            advance();
            if (match(INT)) {
                retp->right = makeNode(INT, getLexeme());
                advance();
            } else if (match(ID)) {
                retp->right = makeNode(ID, getLexeme());
                advance();
            } else if (match(LPAREN)) {
                advance();
                retp->right = expr();
                if (match(RPAREN))
                    advance();
                else
                    error(MISPAREN);
            } else {
                error(NOTNUMID);
            }
        }
    */













    else
    {
        error(NOTNUMID);
    }
    return retp;
}

















/*

// term := factor term_tail
BTNode *term(void) {
    BTNode *node = factor();
    return term_tail(node);
}

// term_tail := MULDIV factor term_tail | NiL
BTNode *term_tail(BTNode *left) {
    BTNode *node = NULL;

    if (match(MULDIV)) {
        node = makeNode(MULDIV, getLexeme());
        advance();
        node->left = left;
        node->right = factor();
        return term_tail(node);
    } else {
        return left;
    }
}

// expr := term expr_tail
BTNode *expr(void) {
    BTNode *node = term();
    return expr_tail(node);
}




// expr_tail := ADDSUB term expr_tail | NiL
BTNode *expr_tail(BTNode *left) {
    BTNode *node = NULL;

    if (match(ADDSUB)) {
        node = makeNode(ADDSUB, getLexeme());
        advance();
        node->left = left;
        node->right = term();
        return expr_tail(node);
    } else {
        return left;
    }
}
*/






// statement := ENDFILE | END | expr END












void err(ErrorType errorNum)
{
    printf("EXIT 1\n");
    if (PRINTERR)
    {
        //fprintf(stderr, "error: ");
        switch (errorNum)
        {
        case MISPAREN:
            //fprintf(stderr, "mismatched parenthesis\n");
            break;
        case NOTNUMID:
            //fprintf(stderr, "number or identifier expected\n");
            break;
        case NOTFOUND:
            //fprintf(stderr, "variable not defined\n");
            break;
        case RUNOUT:
            //fprintf(stderr, "out of memory\n");
            break;
        case NOTLVAL:
            //fprintf(stderr, "lvalue required as an operand\n");
            break;
        case DIVZERO:
            //fprintf(stderr, "divide by constant zero\n");
            break;
        case SYNTAXERR:
            //fprintf(stderr, "syntax error\n");
            break;
        default:
            //fprintf(stderr, "undefined error\n");
            break;
        }
    }

    exit(0);
}


/*============================================================================================
codeGen implementation
============================================================================================*/


int start = 0;
int inass = 0;
char inin[MAXLEN];

int evaluateTree(BTNode *root)
{
    int retval = 0, lv = 0, rv = 0;
    int what = 0;
    int inl = 0, inr = 0;
    //int inid = 0;
    int inidint = 0;
    int isisid = 0;




    if (root != NULL)
    {
        switch (root->data)
        {
        case ID:

            retval = getval(root->lexeme);  //READ INT OUT

            //inid = 1;
            inidint = getplace(root->lexeme);





            break;
        case INT:
            retval = atoi(root->lexeme);  //CHANDE CHAR TO INT
            break;
        case ASSIGN:

            if(root -> left -> data != ID)
            {
                error(NOTNUMID);
            }


            rv = evaluateTree(root->right);
            retval = setval(root->left->lexeme, rv);




            if(root -> right -> data == INT)
            {
                start++;
                printf("MOV r%d %s\n", start, root -> right -> lexeme);
                printf("MOV [%d] r%d\n", getplace(root -> left -> lexeme), start);
            }
            else if(root -> right -> data == ID)
            {
                start++;
                printf("MOV r%d [%d]\n", start, getplace(root -> right -> lexeme));
                printf("MOV [%d] r%d\n", getplace(root -> left -> lexeme), start);
            }
            else
            {
                printf("MOV [%d] r%d\n", getplace(root -> left -> lexeme), start);
            }







            break;
        case ADDSUB:
        case MULDIV:
        case AND:
        case OR:
        case XOR:


            isisid = 0;


             strcpy(inin, root -> left -> lexeme) ;
            //inin = root -> left -> lexeme;

            if(root -> left -> data == ID && haveincdec( root -> right, inin) )
            {
                //root -> isid = 1;
                isisid = 1;

                start++;
                printf("MOV r%d [%d]\n", start, getplace(root->left->lexeme));
            }


            lv = evaluateTree(root->left);
            rv = evaluateTree(root->right);
            if (strcmp(root->lexeme, "+") == 0)
            {
                retval = lv + rv;
                what = 1;
            }
            else if (strcmp(root->lexeme, "-") == 0)
            {
                retval = lv - rv;
                what = 2;
            }
            else if (strcmp(root->lexeme, "*") == 0)
            {
                retval = lv * rv;
                what = 3;
            }
            else if (strcmp(root->lexeme, "/") == 0)
            {
                if (rv == 0 && pass(root -> right) != 1)
                {
                    error(DIVZERO);
                }
                else if(rv == 0 && pass(root -> right) == 1)
                {
                    retval = 0;
                }
                else
                {
                    retval = lv / rv;
                }

                /*
                if(rv == 0)
                {
                    error(DIVZERO);
                }
                else
                {
                    retval = lv / rv;
                }*/


                //retval = lv / rv;
                what = 4;
            }
            else if (strcmp(root->lexeme, "&") == 0)
            {
                retval = lv & rv;
                what = 5;
            }
            else if (strcmp(root->lexeme, "|") == 0)
            {
                retval = lv | rv;
                what = 6;
            }
            else if (strcmp(root->lexeme, "^") == 0)
            {
                retval = lv ^ rv;
                what = 7;
            }

            //inid = 0;
            inass = 0;

            break;
        default:
            retval = 0;
        }


        if(what != 0)
        {



            if(root->left->data == ID && isisid == 0)
            {
                printf("MOV r0 [%d]\n", getplace(root->left->lexeme));
                inl = 1;
            }
            else if(root->left->data == ID && isisid == 1)
            {
                //printf("MOV r%d [%d]\n", start--, getplace(root->left->lexeme));
                inl = 1;
            }
            else if(root->left->data == INT)
            {
                printf("MOV r0 %s\n", root->left->lexeme);
                inl = 1;
            }



            if(root->right->data == ID)
            {
                printf("MOV r1 [%d]\n", getplace(root->right->lexeme));
                inr = 1;
            }
            else if(root->right->data == INT)
            {
                printf("MOV r1 %s\n", root->right->lexeme);
                inr = 1;
            }



            if(isisid == 1)
            {
                start--;
                printf("%s r%d r%d\n", whatsign(what), start, start + 1);


                isisid = 0;
            }
            else
            {
                if(inl == 1 && inr == 1)
                {
                    start++;
                    printf("%s r0 r1\n", whatsign(what));
                    printf("MOV r%d r0\n", start);


                    inl = 0;
                    inr = 0;
                }
                else if(inl == 1 && inr == 0)
                {
                    printf("%s r0 r%d\n", whatsign(what), start);
                    printf("MOV r%d r0\n", start);
                    //printf("MOV r0 r1\n");
                    inl = 0;
                }
                else if(inl == 0 && inr == 1)
                {
                    printf("%s r%d r1\n", whatsign(what), start);
                    inr = 0;
                }
                else if(inl == 0 && inr == 0)
                {
                    start--;
                    printf("%s r%d r%d\n", whatsign(what), start, start + 1);
                }
            }


            what = 0;

        }



    }
    return retval;
}

int getplace(char *str)
{
    int back = 0;

    for (back = 0; back < sbcount; back++)
    {
        if (strcmp(str, table[back].name) == 0)
        {
            //printf("yyyy%s\n",table[back].name);
            break;
        }
    }

    if(back >= sbcount)
    {
        error(NOTFOUND);
    }

    return back * 4;
}

char back[MAXLEN];

char *whatsign(int in)
{
    //char back[MAXLEN];

    if(in == 1)
    {
        strcpy(back,"ADD\0");
    }
    else if(in == 2)
    {
        strcpy(back,"SUB\0");
    }
    else if(in == 3)
    {
        strcpy(back,"MUL\0");
    }
    else if(in == 4)
    {
        strcpy(back,"DIV\0");
    }
    else if(in == 5)
    {
        strcpy(back,"AND\0");
    }
    else if(in == 6)
    {
        strcpy(back,"OR\0");
    }
    else if(in == 7)
    {
        strcpy(back,"XOR\0");
    }



    return back;
}

void printPrefix(BTNode *root)
{
    if (root != NULL)
    {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}

int pass(BTNode* now)
{
    if(now == NULL)
    {
        return 0;
    }

    int pl = 0, pr = 0, back = 0;

    pl = pass(now -> left);
    pr = pass(now -> right);


    if(pl == 1 || pr == 1)
    {
        back = 1;
    }
    else
    {
        if(now -> data == ID)
        {
            back = 1;
        }
    }



    return back;
}

int haveincdec(BTNode* now, char * in)
{
    int pl = 0, pr = 0, back = 0;


    if(now == NULL)
    {
        return 0;
    }

    pl = haveincdec(now -> left, in);
    pr = haveincdec(now -> right, in);


    if(pl == 1 || pr == 1)
    {
        back = 1;
    }
    else
    {
        if(now->left != NULL)
        {
            if(now->data == ASSIGN && strcmp(now->left->lexeme, in) == 0)
            {
                back = 1;
            }
        }

    }



    return back;
}


/*============================================================================================
main
============================================================================================*/

// This package is a calculator
// It works like a Python interpretor
// Example:
// >> y = 2
// >> z = 2
// >> x = 3 * y + 4 / (2 * z)
// It will print the answer of every line
// You should turn it into an expression compiler
// And print the assembly code according to the input

// This is the grammar used in this package
// You can modify it according to the spec and the slide
// statement  :=  ENDFILE | END | expr END
// expr    	  :=  term expr_tail
// expr_tail  :=  ADDSUB term expr_tail | NiL
// term 	  :=  factor term_tail
// term_tail  :=  MULDIV factor term_tail| NiL
// factor	  :=  INT | ADDSUB INT |
//		   	      ID  | ADDSUB ID  |
//		   	      ID ASSIGN expr |
//		   	      LPAREN expr RPAREN |
//		   	      ADDSUB LPAREN expr RPAREN

int main()
{
    initTable();
    //printf(">> ");
    while (1)
    {
        start = 1;
        statement();
    }
    return 0;
}

