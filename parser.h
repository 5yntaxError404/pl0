#include "ast.h"
#include "utilities.h"
#include "lexer.h"
#include "token.h"
#include "unparser.h"
#include "id_attrs.h"
#include "reserved.h"
#include "lexer_output.h"
#include "scope_check.h"
#include "scope_symtab.h"
#include "type_attrs.h"

// Advances the lexer by fetching the next token from the input source
static void advance();

// Checks if the next token in the input source is of the expected token type and consumes it.
// If invalid token, throws error.
void eat(token_type token);

// Parses the input source and generates an AST for the entire program.
AST *parseProgram();

// Parses the variable declarations and generates an AST for them.
static AST_list parseVars();

// Generates an AST for a variable declaration.
static AST_list parseIdents_VAR(var_decl_t var);

// Parses the constant declarations and generates an AST for them.
static AST_list parseConsts();



// inserts the given AST_list into the linked list starting from head and updating the tail.
static void AST_insert_back(AST_list *head, AST_list *tail, AST_list list);

// Parses a statement and generates an AST for it.
AST *parseStmt();


// Parses an expression and generates an AST for it.
AST *parseExpression();

static AST *parseTerm();
static AST *parseAddSubTerm();
static AST *parseMultDivFactor();


// Parses a factor and generates an AST for it.
static AST *parseFactor();

// Parses a condition and generates an AST for it.
AST *parseCondition();

// Parses an if statement and generates an AST for it.
AST *parseIfStmt();

// Parses an assignment statement and generates an AST for it.
static AST* parseAssignStmt();

// Parses a begin statement and generates an AST for it.
static AST_list parseBeginStmt();

// Parses a while loop statement and generates an AST for it.
AST *parseWhileStmt();

// Parses a read statement and generates an AST for it.
AST* parseReadStmt();

// Parses a write statement and generates an AST for it.
AST *parseWriteStmt();

// Parses a skip statement and generates an AST for it.
AST *parseSkipStmt();

// Checks if the given token_type is a relational operator.
static bool rel_op_check(token_type token);

// Returns the op_expr_t structure containing the operator data from the given op_expr_ast AST node.
op_expr_t getOpData(AST* op_expr_ast);


// Parses an identifier expression and generates an AST for it.
static AST *parseIdentExpr();

// Parses a parenthesized expression and generates an AST for it.
static AST *parseParenExpr();

// Parses a numeric expression
static AST *parseNumExpr();

// parses a term in an arithmetic expression. 
// It looks for a factor, possibly followed by zero or more multiplicative operators (multsym or divsym) and factors, and 
// constructs an AST node representing the term.
static AST *parseTerm();

// Parses a signed term in an arithmetic expression. 
// It looks for a sign token (either plussym or minussym) followed by a term, and 
// constructs an AST node representing the signed term. The function returns a pointer to the constructed AST node.
static AST *parseSign();

void printList(AST_list);

void parser_open(const char *filename);

void parser_close();

static bool is_stmt_beginning_token(token t);

static AST_list parseIdents_CONST();

static void usage(const char*);
