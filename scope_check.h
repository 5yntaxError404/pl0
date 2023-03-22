// Daniel Correa and David Umanzor - scope_check.h
// Cited From Float Language
// Talked conceptually with Group 25 - HagMik

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

// Builds sym table for the AST prog and checks the given AST for double
//  declarations or uses of identifiers that were never delcared.
extern void scope_check_program(AST *prog);

// Puts given name, which is to be delcared with id_kind vars, and has its declaration at the floc
// into the current scopes symbl table at the offset scope_Size()
static void add_ident_to_scope(const char *name, id_kind vars, file_location f_locate);

// builds sym table and checks the declarations in vars
extern void scope_check_varDecls(AST *vds);

// checks var delcarations and adds it to the current scopes sym table, or produces err if names
// been declared already. 
void scope_check_varDecl(AST *var);

// Puts given name, which is to be delcared with id_kind consts, and has its declaration at the floc
// into the current scopes symbl table at the offset scope_Size()
static void add_const_to_scope(const char *name, id_kind consts, file_location f_locate);

// builds sym table and checks the declarations in consts
void scope_check_constDecls(AST_list consts);

// checks const delcarations and adds it to the current scopes sym table, or produces err if names
// been declared already. 
void scope_check_constDecl(AST *consts);

// check the statement to make sure that all idents referenced in it have been delcared, otherwise, 
// error is returned 
extern void scope_check_stmt(AST *stmt);

// check the statement to make sure that all idents referenced in it have been delcared, otherwise, 
// error is returned 
void scope_check_whileStmt(AST *stmt);

// check the statement to make sure that all idents referenced in it have been delcared, otherwise, 
// error is returned 
extern void scope_check_assignStmt(AST *stmt);

// check the statement to make sure that all idents referenced in it have been delcared, otherwise, 
// error is returned 
extern void scope_check_beginStmt(AST *stmt);

// check the statement to make sure that all idents referenced in it have been delcared, otherwise, 
// error is returned 
extern void scope_check_ifStmt(AST *stmt);

// check the statement to make sure that all idents referenced in it have been delcared, otherwise, 
// error is returned 
extern void scope_check_readStmt(AST *stmt);

// check the statement to make sure that all idents referenced in it have been delcared, otherwise, 
// error is returned 
extern void scope_check_writeStmt(AST *stmt);

// check the expresion to make sure that all idents referenced in it have been declared, otherwise,
// error is returned
void scope_check_op_expr(AST *exp);

// check the expresion to make sure that all idents referenced in it have been declared, otherwise,
// error is returned
extern void scope_check_expr(AST *exp);

// check that the given name has been declared,
// if not, then produce an error using the file_location (floc) given.
extern void scope_check_ident(file_location floc, const char *name);

// check the expresion to make sure that all idents referenced in it have been declared, otherwise,
// error is returned
extern void scope_check_bin_expr(AST *exp);

// check the condition to make sure that all idents referenced in it have been declared, otherwise,
// error is returned
void scope_check_bin_cond(AST *cond);

// check the condition to make sure that all idents referenced in it have been declared, otherwise,
// error is returned
void scope_check_odd_cond(AST *cond);

// check the condition to make sure that all idents referenced in it have been declared, otherwise,
// error is returned
void scope_check_cond(AST *cond);

#endif
