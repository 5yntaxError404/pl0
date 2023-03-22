#include "type_attrs.h"
#include "scope_check.h"
#include "scope_symtab.h"


// Return a string representing the var_type given
const char *vt2str(id_kind vt)
{
    if (vt == variable) {
	return kind2str(1);
    } else {
	return kind2str(0);
    }
}