
/* $Id: scope_check.h,v 1.2 2023/02/22 03:33:43 leavens Exp $ */
#ifndef _SCOPE_CHECK_H
#define _SCOPE_CHECK_H
#include "ast.h"
#include "utilities.h"
#include "lexer.h"
#include "token.h"
#include "scope_symtab.h"
#include "id_attrs.h"
#include "ast.h"
#include "file_location.h"
#include "utilities.h"
#include "type_attrs.h"
// Build the symbol table for the given program AST
// and Check the given program AST for duplicate declarations
// or uses of identifiers that were not declared
extern void scope_check_program(AST *prog);

// build the symbol table and check the declarations in vds
extern void scope_check_varDecls(AST *vds);

// check the var declaration vd
// and add it to the current scope's symbol table
// or produce an error if the name has already been declared
void scope_check_varDecl(AST *var);

static void add_const_to_scope(const char *name, id_kind consts, file_location f_locate);
void scope_check_constDecls(AST_list consts);
void scope_check_constDecl(AST *consts);
// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
extern void scope_check_stmt(AST *stmt);

void scope_check_whileStmt(AST *stmt);

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
extern void scope_check_assignStmt(AST *stmt);

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
extern void scope_check_beginStmt(AST *stmt);

static void add_ident_to_scope(const char *name, id_kind vars, file_location f_locate);

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
extern void scope_check_ifStmt(AST *stmt);

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
extern void scope_check_readStmt(AST *stmt);

// check the statement to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
extern void scope_check_writeStmt(AST *stmt);

void scope_check_op_expr(AST *exp);

// check the expresion to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
extern void scope_check_expr(AST *exp);

// check that the given name has been declared,
// if not, then produce an error using the file_location (floc) given.
extern void scope_check_ident(file_location floc, const char *name);

// check the expression (exp) to make sure that
// all idenfifiers referenced in it have been declared
// (if not, then produce an error)
extern void scope_check_bin_expr(AST *exp);

void scope_check_bin_cond(AST *cond);
void scope_check_odd_cond(AST *cond);
void scope_check_cond(AST *cond);

#endif
