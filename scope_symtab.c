// Daniel Correa and David Umanzor
// Cited From Float Language
// Talked conceptually with Group 25 - HagMik
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "scope_symtab.h"

typedef struct {
    const char *id;
    id_attrs *attrs;
} symtab_assoc_t;

typedef struct scope_symtab_s {
    unsigned int size;
    symtab_assoc_t *entries[MAX_SCOPE_SIZE];
} scope_symtab_t;

// The current scope
static scope_symtab_t *symtab = NULL;

// Allocates the memory for a new scope symbol table
// returns an error message if there is no space and exits the code
static scope_symtab_t * scope_create()
{
    scope_symtab_t *new_scope =  malloc(sizeof(scope_symtab_t));
    if (new_scope == NULL) 
	    bail_with_error("No space for new scope_symtab_t!");
    
    new_scope->size = 0;

    for (int j = 0; j < MAX_SCOPE_SIZE; j++) 
	    new_scope->entries[j] = NULL;
    
    return new_scope;
}

// initialize the symbol table for the current scope
void scope_initialize()
{
    // assigns the scope to the global symtab
    symtab = scope_create();
}

// Return the current scope's next offset to use for allocation,
// which is the size of the current scope.
unsigned int scope_size()
{
    return symtab->size;
}

// Is the scope at max size
bool scope_full()
{
    return scope_size() >= MAX_SCOPE_SIZE;
}

// Add an association from the given name to the given id
static void scope_add(symtab_assoc_t *assoc)
{
    symtab->entries[symtab->size] = assoc;
    symtab->size++;
}

// Modify the current scope symbol table to hold a new association
// if there is not enough room return and error and exit the code.
void scope_insert(const char *name, id_attrs *attrs)
{   
    symtab_assoc_t *new_assoc = malloc(sizeof(symtab_assoc_t));
    if (new_assoc == NULL) 
	    bail_with_error("No space for association!");
    
    new_assoc->id = name;
    new_assoc->attrs = attrs;
    scope_add(new_assoc);
}

// Is the given name associated with some attributes in the current scope?
bool scope_defined(const char *name)
{
    // assert(symtab != NULL);
    // assert(name != NULL);
    return scope_lookup(name) != NULL;
}

// Return the attribute attached to some name in the current scope
// or NULL if none can be found
id_attrs *scope_lookup(const char *name)
{
    int i;
    
    for (i = 0; i < symtab->size; i++) 
        if (strcmp(symtab->entries[i]->id, name) == 0) 
            return symtab->entries[i]->attrs;

    return NULL;
}
