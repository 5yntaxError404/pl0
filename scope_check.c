// Daniel Correa and David Umanzor - scope_check.c
// Cited From Float Language
// Talked conceptually with Group 25 - HagMik

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scope_check.h"
#include "scope_symtab.h"

// Builds sym table for the AST prog and checks the given AST for double
//  declarations or uses of identifiers that were never delcared.
void scope_check_program(AST *prog)
{
    scope_check_constDecls(prog->data.program.cds);
    scope_check_varDecls(prog->data.program.vds);
    scope_check_stmt(prog->data.program.stmt);
}

// Puts given name, which is to be delcared with id_kind vars, and has its declaration at the floc
// into the current scopes symbl table at the offset scope_Size()
static void add_ident_to_scope(const char *name, id_kind vars, file_location f_locate)
{
    id_attrs *attrs = scope_lookup(name);
    
    if (attrs != NULL)
        general_error(f_locate,"%s \"%s\" is already declared as a %s",kind2str(vars),name,kind2str(attrs->kind));
	    
    else 
        scope_insert(name, create_id_attrs(f_locate, vars, scope_size()));        
}

// builds sym table and checks the declarations in vars
void scope_check_varDecls(AST_list vars)
{
    while (!ast_list_is_empty(vars)) 
    {
        scope_check_varDecl(ast_list_first(vars));
        vars = ast_list_rest(vars);
    }
}

// checks var delcarations and adds it to the current scopes sym table, or produces err if names
// been declared already. 
void scope_check_varDecl(AST *var)
{
    add_ident_to_scope(var->data.var_decl.name,1,var->file_loc);
}

// Puts given name, which is to be delcared with id_kind consts, and has its declaration at the floc
// into the current scopes symbl table at the offset scope_Size()
static void add_const_to_scope(const char *name, id_kind consts, file_location f_locate)
{
    id_attrs *attrs = scope_lookup(name);
    
    if (attrs != NULL)
        general_error(f_locate,"%s \"%s\" is already declared as a %s",kind2str(consts),name,kind2str(attrs->kind)); 
	    
    else 
        scope_insert(name, create_id_attrs(f_locate, consts, scope_size()));        
}

// builds sym table and checks the declarations in consts
void scope_check_constDecls(AST_list consts)
{
    while (!ast_list_is_empty(consts)) 
    {
        scope_check_constDecl(ast_list_first(consts));
        consts = ast_list_rest(consts);
    }
}

// checks const delcarations and adds it to the current scopes sym table, or produces err if names
// been declared already. 
void scope_check_constDecl(AST *consts)
{
    add_ident_to_scope(consts->data.const_decl.name,0,consts->file_loc);
}

// check the statement to make sure that all idents referenced in it have been delcared, otherwise, 
// error is returned 
void scope_check_stmt(AST *stmt)
{
    switch (stmt->type_tag) 
    {
        case assign_ast:
            scope_check_assignStmt(stmt);
            break;
        case begin_ast:
            scope_check_beginStmt(stmt);
            break;
        case if_ast:
            scope_check_ifStmt(stmt);
            break;
        case read_ast:
            scope_check_readStmt(stmt);
            break;
        case write_ast:
            scope_check_writeStmt(stmt);
            break;
        case skip_ast:
            break;
        case while_ast:
            scope_check_whileStmt(stmt);
            break;
        default:
            bail_with_error("Call to scope_check_stmt with an AST that is not a statement!");
            break;
    }
}

// check the statement to make sure that all idents referenced in it have been delcared, otherwise, 
// error is returned 
void scope_check_whileStmt(AST *stmt)
{
    scope_check_stmt(stmt->data.while_stmt.stmt);
    scope_check_expr(stmt->data.while_stmt.cond);
}

// check the statement to make sure that all idents referenced in it have been delcared, otherwise, 
// error is returned 
void scope_check_assignStmt(AST *stmt)
{
    scope_check_ident(stmt->file_loc, stmt->data.assign_stmt.name);
    scope_check_expr(stmt->data.assign_stmt.exp);
}

// check the statement to make sure that all idents referenced in it have been delcared, otherwise, 
// error is returned 
void scope_check_beginStmt(AST *stmt)
{
    AST_list stmts = stmt->data.begin_stmt.stmts;
    while (!ast_list_is_empty(stmts)) 
    {
        scope_check_stmt(ast_list_first(stmts));
        stmts = ast_list_rest(stmts);
    }
}

// check the statement to make sure that all idents referenced in it have been delcared, otherwise, 
// error is returned 
void scope_check_ifStmt(AST *stmt)
{
    scope_check_cond(stmt->data.if_stmt.cond);
    scope_check_stmt(stmt->data.if_stmt.thenstmt);
    scope_check_stmt(stmt->data.if_stmt.elsestmt);  
}

// check the statement to make sure that all idents referenced in it have been delcared, otherwise, 
// error is returned 
void scope_check_readStmt(AST *stmt)
{
    scope_check_ident(stmt->file_loc, stmt->data.read_stmt.name);
}

// check the statement to make sure that all idents referenced in it have been delcared, otherwise, 
// error is returned 
void scope_check_writeStmt(AST *stmt)
{
    scope_check_expr(stmt->data.write_stmt.exp);
}

// check the expresion to make sure that all idents referenced in it have been declared, otherwise,
// error is returned
void scope_check_expr(AST *exp)
{
    switch (exp->type_tag) 
    {
        case ident_ast:
            scope_check_ident(exp->file_loc, exp->data.ident.name);
            break;
        case bin_expr_ast:
            scope_check_bin_expr(exp);
            break;
        case op_expr_ast:
            scope_check_op_expr(exp);
            break;
        case number_ast:
            break;
        default:
            //bail_with_error("Unexpected type_tag (%d) in scope_check_expr (for line %d, column %d)!",
            //   exp->type_tag, exp->file_loc.line, exp->file_loc.column);
            break;
    }
}

// check the expresion to make sure that all idents referenced in it have been declared, otherwise,
// error is returned
void scope_check_op_expr(AST *exp)
{
    scope_check_expr(exp->data.op_expr.exp);
}

// check that the given name has been declared, if not, then produce an error using the floc given.
void scope_check_ident(file_location floc, const char *name)
{
    if (!scope_defined(name)) 
	    general_error(floc, "identifer \"%s\" is not declared!", name);
}

// check the expresion to make sure that all idents referenced in it have been declared, otherwise,
// error is returned
void scope_check_bin_expr(AST *exp)
{
    scope_check_expr(exp->data.bin_expr.leftexp);
    scope_check_expr(exp->data.bin_expr.rightexp);
}

// check the condition to make sure that all idents referenced in it have been declared, otherwise,
// error is returned
void scope_check_cond(AST *cond)
{
    switch(cond->type_tag)
    {
        case odd_cond_ast:
            scope_check_odd_cond(cond);
            break;
        case bin_cond_ast:
            scope_check_bin_cond(cond);
            break;
        default:
             bail_with_error("Unexpected type_tag (%d) in scope_check_cond (for line %d, column %d)!",
                cond->type_tag, cond->file_loc.line, cond->file_loc.column);
    }
}

// check the condition to make sure that all idents referenced in it have been declared, otherwise,
// error is returned
void scope_check_odd_cond(AST *cond)
{
    scope_check_expr(cond->data.odd_cond.exp);
}

// check the condition to make sure that all idents referenced in it have been declared, otherwise,
// error is returned
void scope_check_bin_cond(AST *cond)
{
    scope_check_expr(cond->data.bin_cond.leftexp);
    scope_check_expr(cond->data.bin_cond.rightexp);
}