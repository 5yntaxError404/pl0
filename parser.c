// Daniel Correa and David Umanzor
// Parser and Declaration Checker.c
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



//static FILE *input_file = NULL;

// The input file's name
static char *filename = NULL;
static token tok;
static unsigned int scope_offset;

static void usage(const char* cmdname)
{
    fprintf(stderr, "Usage: %s [-l] code-filename\n", cmdname);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{   
    
    const char *cmdname = argv[0];
    int filename_index = 1;
    bool produce_lexer_output = false;
    
    if ( argc >= 2 && strcmp(argv[1],"-l") == 0 ) 
    {
        produce_lexer_output = true;
        filename_index = 2;
    }
    else if ( argc == 2 )
        filename_index = 1;
    

    if (produce_lexer_output) {
        filename = argv[filename_index];
        lexer_open(filename);
        lexer_output();
        lexer_close();
        
    }
    
    else
    {
        
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

static void advance() 
{
    if (!lexer_done())
        tok = lexer_next();
}

void eat(token_type tt) 
{
    token_type expected[1] = {tt}; 

    if (tok.typ == tt)
        advance();
    else   
        parse_error_unexpected(expected, 1, tok);
 }

// BEGIN AST LIST FUNCS

AST *parseProgram()
{
    
    AST_list consts = parseConsts();
    AST_list vars = parseVars();
    AST *stmt = parseStmt();
    
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
    AST_list varHead = ast_list_empty_list(), varTail = ast_list_empty_list();

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
    AST_list constHead = ast_list_empty_list(), constTail = ast_list_empty_list();

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

    AST_list constHead = ast_list_singleton(ast_const_def(ident_tok,ident_tok.text,const_val.value));
    AST_list constTail = constHead;

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

// BEGIN PARSE STMTS
AST *parseStmt()
{
    AST *ret = NULL;
    
            
    switch (tok.typ) 
    {
        case identsym:
            ret = parseAssignStmt(); // NOT FINISHED
            break;
        case beginsym:
            ret = parseBeginStmt(); // NOT FINISHED
            break;
        case ifsym:
            ret = parseIfStmt(); // NOT FINISHED
            break;
        case whilesym:
            ret = parseWhileStmt(); // NOT FINISHED
            break;
        case readsym:
            ret = parseReadStmt(); // NOT FINISHED
            break;
        case writesym:
            ret = parseWriteStmt(); // NOT FINISHED
            break;
        case skipsym:
            ret = parseSkipStmt(); // NOT FINISHED
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



// *** PARSING EXPRESSIONS ***


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


static AST *parseTerm() // its gonna say unused until we figure out neg ASTs
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

static AST *parseNumExpr()
{
    token num_tok;
    int val;

    if(tok.typ == minussym)
    {
        eat(minussym);
        num_tok = tok;
        eat(numbersym);
        val = num_tok.value;
        val = -val;
    }

    else
    {
        num_tok = tok;
        eat(numbersym);
        val = num_tok.value;
    }
    return ast_number(num_tok,val);
}

static AST *parseParenExpr() // may wanna check this one later
{
    token par_tok = tok;
    eat(lparensym);
    AST *ret = parseExpression();
    eat(rparensym);
    ret->file_loc = token2file_loc(par_tok);
    return ret;
}

// END OF PARSE EXPR FUNCTIONS

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

AST *parseIfStmt() // In theory this works, Dunno its Leavens code - David
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

AST *parseAssignStmt() // Unfinished
{
    token ident_token = tok;
    eat(identsym);
    eat(becomessym);

    AST* exp = parseExpression();
    
    return ast_assign_stmt(ident_token, ident_token.text, exp);
}

#define STMTBEGINTOKS 7
static token_type can_begin_stmt[STMTBEGINTOKS] =
	    {identsym, whilesym, ifsym, readsym, writesym, beginsym, skipsym};

AST *parseBeginStmt() // Unfinished
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

static bool is_stmt_beginning_token(token t)
{
    
    for (int i = 0; i < STMTBEGINTOKS; i++) {
	if (t.typ == can_begin_stmt[i]) {
	    return true;
	}
    }
    return false;
}

AST *parseWhileStmt() // Unfinished
{
    token while_token = tok;
    eat(whilesym);
    AST *cond = parseCondition();
    eat(dosym);
    AST *body = parseStmt();
    
    return ast_while_stmt(while_token, cond, body);
}

AST *parseReadStmt() // Unfinished
{
    token read = tok;
    eat(readsym);
    const char *name = tok.text;
    eat(identsym);
    
    return ast_read_stmt(read, name);
}

AST *parseWriteStmt() 
{
    token write = tok;
    eat(writesym);

    AST *exp = parseExpression();
    
    return ast_write_stmt(write, exp);
}

AST *parseSkipStmt() // Unfinished
{
    token skip = tok;
    eat(skipsym);

    return ast_skip_stmt(skip);
}

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