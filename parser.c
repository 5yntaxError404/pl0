// Daniel Correa and David Umanzor
// Parser and Declaration Checker.c
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The input file's name

static char *filename = NULL;
static token tok;
static unsigned int scope_offset;

int main(int argc, char *argv[])
{   
    const char *cmdname = argv[0];
    int filename_index = 1;
    bool produce_lexer_output = false;
    
    // based on number of arguments the filename index will differ
    if ( argc >= 2 && strcmp(argv[1],"-l") == 0 ) 
    {
        produce_lexer_output = true;
        filename_index = 2;
    }
    else if ( argc == 2 )
        filename_index = 1;

    // if true use Lexer functions
    if (produce_lexer_output) {
        filename = argv[filename_index];
        lexer_open(filename);
        lexer_output();
        lexer_close();
    }
    else
    {
        // otherwise run parser like normal
        filename = argv[1];
        parser_open(filename);
        AST *progast = parseProgram();
        parser_close();
        unparseProgram(stdout,progast);

        scope_initialize();
        scope_check_program(progast);
    }
    return EXIT_SUCCESS;
}

// ---------- PARSER OPERATION FUNCTIONS ----------

// initialize the parser to work on the given file
void parser_open(const char *filename)
{ 
    lexer_open(filename);
    tok = lexer_next();
    //scope_offset = 0;
}

// finish using the parser
void parser_close()
{
    lexer_close();
}

// advance the parser and update the token
static void advance() 
{
    if (!lexer_done())
        tok = lexer_next();
}

// Checks if the next token in the input source is of the expected token type and consumes it.
// If invalid token, throws error.
void eat(token_type tt) 
{
    token_type expected[1] = {tt}; 

    if (tok.typ == tt)
        advance();
    else   
        parse_error_unexpected(expected, 1, tok);
 }

// ------------- BEGIN AST LIST FUNCS -------------------------

AST *parseProgram()
{
    // create AST lists of constants, vars, and an AST stmts for processing
    AST_list consts = parseConsts();
    AST_list vars = parseVars();
    AST *stmt = parseStmt();
    // after completed eat end of file sym
    eat(periodsym);
    eat(eofsym);

    file_location f_locate;
    if (!ast_list_is_empty(vars)) 
    {
        if (ast_list_first(vars)->type_tag == var_decl_ast) 
            f_locate = ast_list_first(vars)->file_loc;
	    else 
	        bail_with_error("Bad AST for var declarations");
	}
    else 
	    f_locate = stmt->file_loc;

    return ast_program(filename,f_locate.line,f_locate.column,consts,vars,stmt);
}


static AST_list parseVars()
{
    // create a empty list for the head and tail of the variable list
    AST_list varHead = ast_list_empty_list(), varTail = ast_list_empty_list();
    // while there are varsym tokens keep adding them inserting at the back of the list
    while (tok.typ == varsym)
    {
        AST_list varASTs;
        var_decl_t var;
        while (tok.typ == varsym)
        {
            eat(varsym);
            var.name = tok.text;
            varASTs = parseIdents_VAR(var);
            eat(semisym);
            AST_insert_back(&varHead,&varTail,varASTs);
        }
    }
    return varHead;
}

static AST_list parseIdents_VAR(var_decl_t var)
{
    // create an list for indents based on the var head and tail
    token ident_tok = tok;
    eat(identsym);
    AST_list varHead = ast_list_singleton(ast_var_decl(ident_tok, ident_tok.text));
    AST_list varTail = varHead;

    while (tok.typ == commasym)
    {
        eat(commasym); // will lexer next over commas and continue.
        ident_tok = tok;
        eat(identsym);
        AST *vars = ast_var_decl(ident_tok,ident_tok.text);
        AST_insert_back(&varHead,&varTail,ast_list_singleton(vars));
    }
    return varHead;    
}

static AST_list parseConsts()
{
    // create an empty list for the head and tail of the constant list
    AST_list constHead = ast_list_empty_list(), constTail = ast_list_empty_list();
    // while there are constsym tokens keep adding them inserting at the back of the list
    while (tok.typ == constsym)
    {
        eat(constsym);
        AST_list constASTs; 
        constASTs = parseIdents_CONST();
        eat(semisym);
        AST_insert_back(&constHead,&constTail,constASTs);
    }
    return constHead;
}

static AST_list parseIdents_CONST()
{
    token ident_tok = tok;
    eat(identsym);
    eat(eqsym);

    token const_val = tok;
    eat(numbersym);
    
    // creates head and tail of the AST_list for const within the indents, setting the head equal to the tail
    AST_list constHead = ast_list_singleton(ast_const_def(ident_tok,ident_tok.text,const_val.value));
    AST_list constTail = constHead;

    // while token type is equal to the commasym, build the rest of the AST_list inserting from the back
    while (tok.typ == commasym)
    {
        eat(commasym);
        ident_tok = tok;
        eat(identsym);
        eat(eqsym);

        const_val = tok;
        eat(numbersym);
        AST *new = ast_list_singleton(ast_const_def(ident_tok,ident_tok.text,const_val.value));

        AST_insert_back(&constHead,&constTail,new);
    }
    return constHead;    
}

// This function inserts the ast to the back of the list making the oldest AST the head and the newest AST the tail
static void AST_insert_back(AST_list *head, AST_list *tail, AST_list list)
{
    if (!ast_list_is_empty(*head))
    {
        ast_list_splice(*tail,list);
        *tail = ast_list_last_elem(list);
    }
    else
    {
        *head = list;
        *tail = ast_list_last_elem(list);
    }
}

// ---------------- BEGIN PARSE STMTS ------------------

AST *parseStmt()
{
    AST *ret = NULL;

    // Check for a match in any of the valid token types
    // Parse based on token type found otherwise exit from invalid token
    switch (tok.typ) 
    {
        case identsym:
            ret = parseAssignStmt();
            break;
        case beginsym:
            ret = parseBeginStmt();
            break;
        case ifsym:
            ret = parseIfStmt();
            break;
        case whilesym:
            ret = parseWhileStmt();
            break;
        case readsym:
            ret = parseReadStmt();
            break;
        case writesym:
            ret = parseWriteStmt();
            break;
        case skipsym:
            ret = parseSkipStmt();
            break;

        // ... other cases of expected tokens...
        case thensym:
            ret = parseIfStmt();
            break;
        default:
        {
            token_type expected[7] = {identsym, beginsym, ifsym, whilesym, readsym, writesym, skipsym};
            parse_error_unexpected(expected, 7, tok);
        }
        break;
    }
    return ret;
}

// ------------------------- PARSING EXPRESSIONS ------------------------

AST *parseExpression()
{
   token exp_tok = tok;
   AST *expr = parseTerm();
   while (tok.typ == plussym || tok.typ == minussym)
   {
        AST *right = parseAddSubTerm();
        expr = ast_bin_expr(exp_tok,expr,getOpData(right).arith_op,getOpData(right).exp);
   }
   return expr;
}

// parses a term in an arithmetic expression. 
// It looks for a factor, possibly followed by zero or more multsym or divsym operators and factors, and 
// constructs an AST node representing the term.
static AST *parseTerm()
{
    token exp_tok = tok;
    AST *factor = parseFactor();
    AST *expr = factor;
    while (tok.typ == multsym || tok.typ == divsym)
    {
        AST *right = parseMultDivFactor();
        expr = ast_bin_expr(exp_tok,expr,getOpData(right).arith_op,getOpData(right).exp);
    }
    return expr;
} 

// parse for addition or subtraction operations and create AST operation expression, 
// if non found invalid parse operation
static AST *parseAddSubTerm()
{
    token op = tok;
    AST *expr;

    switch(tok.typ)
    {
        case plussym:
            eat(plussym);
            expr = parseTerm();
            return ast_op_expr(op,addop,expr);
            break;
        case minussym:
            eat(minussym);
            expr = parseTerm();
            return ast_op_expr(op,subop,expr);
            break;
        default:
        {
            token_type expected[2] = {plussym, minussym};
            parse_error_unexpected(expected, 2, tok);
        }
            break;
    }
    return (AST *) NULL;
}

// parse for multiplication or division operations and create AST operation expression, 
// if non found invalid parse operation
static AST *parseMultDivFactor()
{
    token op = tok;
    
    AST *expr;

    switch(tok.typ)
    {
        case multsym:
            eat(multsym);
            expr = parseFactor();
            return ast_op_expr(op,multop,expr);
            break;
        case divsym:
            eat(divsym);
            expr = parseFactor();
            return ast_op_expr(op,divop,expr);
            break;
        default:
        {
            token_type expected[2] = {multsym, divsym};
            parse_error_unexpected(expected, 2, tok);
        }
        break;
    }
    return (AST *) NULL;
}

static AST *parseFactor()
{
// check token type for indents, if or while, numbers, or minus symbols
    switch (tok.typ) 
    {
        case identsym:
            return parseIdentExpr();
            break;
        case lparensym:
            return parseParenExpr(); // for if and whiles. 
            break;
        case numbersym:
            return parseNumExpr();
            break;
        case minussym:
            return parseNumExpr();
            break;
        default:
        {
            token_type expected[3] = {identsym, lparensym, numbersym};
            parse_error_unexpected(expected, 3, tok);
        }    
        break;
    }
    // The following should never execute
    return (AST *)NULL;
}

static AST *parseIdentExpr()
{
    token id_tok = tok;
    eat(identsym);
    return ast_ident(id_tok,id_tok.text);
}

// parse number expression and build an AST
static AST *parseNumExpr()
{
    token num_tok;
    int val;

    if(tok.typ == minussym) // based on if its a minus sym set the value to negative
    {
        eat(minussym);
        num_tok = tok;
        eat(numbersym);
        val = num_tok.value;
        val = -val;
    }
    else // otherwise eat the numbersym and store like normal
    {
        num_tok = tok;
        eat(numbersym);
        val = num_tok.value;
    }
    return ast_number(num_tok,val);
}


static AST *parseParenExpr()
{
    token par_tok = tok;
    eat(lparensym);
    AST *ret = parseExpression();
    eat(rparensym);
    ret->file_loc = token2file_loc(par_tok);
    return ret;
}

// ------------------------ END OF PARSE EXPR FUNCTIONS ----------------------------

AST *parseCondition()
{
    AST *ret = NULL;
    token conditional = tok;
    // we need to collect the var being passed in. Typical if stmts are like

    if ((tok.typ) == oddsym)
    {
        eat(oddsym);
        AST *oddExp =  parseExpression();
        ret = ast_odd_cond(tok, oddExp);
    } 
    else
    {
        AST *L_Exp = parseExpression();
        rel_op operator;

        switch (tok.typ)
        {
            case eqsym:
                operator = eqop;
                eat(eqsym);
                break;
            case lessym:
                operator = ltop;
                eat(lessym);  
                break;
            case leqsym:
                operator = leqop;
                eat(leqsym);
                break;
            case gtrsym:
                operator = gtop;
                eat(gtrsym);
                break;
            case geqsym:
                operator = geqop;
                eat(geqsym);
                break;
            case neqsym:
                operator = neqop;
                eat(neqsym);
                break;
            default:
                return NULL;
                break;
        }
        AST *R_Exp = parseExpression();
        ret = ast_bin_cond(tok,L_Exp,operator,R_Exp);
    }
    return ret;    
}

// parse if statement and build an AST
AST *parseIfStmt()
{
    token ift = tok;
    eat(ifsym);
    AST *cond = parseCondition();
    eat(thensym);
    AST *thenstmt = parseStmt();
    eat(elsesym);
    AST *elsestmt = parseStmt();
    return ast_if_stmt(ift, cond, thenstmt, elsestmt);
}

// parse indentity statement and build an AST
AST *parseAssignStmt()
{
    token ident_token = tok;
    eat(identsym);
    eat(becomessym);

    AST* exp = parseExpression();
    
    return ast_assign_stmt(ident_token, ident_token.text, exp);
}

// Valid tokens that can begin a statement
#define STMTBEGINTOKS 7
static token_type can_begin_stmt[STMTBEGINTOKS] =
	    {identsym, whilesym, ifsym, readsym, writesym, beginsym, skipsym};

// parse begin statement returning a begin AST
AST *parseBeginStmt()
{
    token begin_tok = tok;
    eat(beginsym);
    
    AST_list stmts = ast_list_singleton(parseStmt());
    AST_list last = stmts;
    
    while(tok.typ == semisym)
    {
        eat(semisym);
        AST *stmt = parseStmt();
        AST_insert_back(&stmts, &last, ast_list_singleton(stmt));
    }
    
    eat(endsym);
    AST *begin_ret = ast_begin_stmt(begin_tok,stmts);

    return begin_ret;
}

// return true if token is a valid beginning token, otherwise false
static bool is_stmt_beginning_token(token t)
{
    
    for (int i = 0; i < STMTBEGINTOKS; i++) {
	if (t.typ == can_begin_stmt[i]) {
	    return true;
	}
    }
    return false;
}

// parse while statement and build an AST
AST *parseWhileStmt()
{
    token while_token = tok;
    eat(whilesym);
    AST *cond = parseCondition();
    eat(dosym);
    AST *body = parseStmt();
    
    return ast_while_stmt(while_token, cond, body);
}

// parse read statement and build an AST
AST *parseReadStmt()
{
    token read = tok;
    eat(readsym);
    const char *name = tok.text;
    eat(identsym);
    
    return ast_read_stmt(read, name);
}

// parse write statement and build an AST
AST *parseWriteStmt() 
{
    token write = tok;
    eat(writesym);

    AST *exp = parseExpression();
    
    return ast_write_stmt(write, exp);
}

// parse skip statement and build an AST
AST *parseSkipStmt()
{
    token skip = tok;
    eat(skipsym);

    return ast_skip_stmt(skip);
}

// relational operation checker, if token matches a relational operation return true
static bool rel_op_check(token_type tok)
{
    for (int i = eqop; i <= geqop; i++) // loop through the enumerations
        if (tok == i)
            return true;
    return false; // Never Found
}

op_expr_t getOpData(AST *op_expr_ast)
{
    return op_expr_ast->data.op_expr;
}