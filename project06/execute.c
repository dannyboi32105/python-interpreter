/*execute.c

The purpose of this file is to implement the execute() function
   for the nuPython interpreter.  The execute() function takes a
   program graph and a RAM memory, and executes the statements in
   the program graph.  If a semantic error occurs (e.g. type error),
   an error message is output, execution stops, and the function
   returns.
*/

//
// >>
//
// <<Daniel Solomon >>
// << Northwestern University>>
// << CS 211>>
// << Spring Quarter>>
// 
// 
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false
#include <string.h>
#include <assert.h>
#include <math.h>

#include "programgraph.h"
#include "ram.h"
#include "execute.h"



//
// Public functions:
//

//
// execute
//
// Given a nuPython program graph and a memory, 
// executes the statements in the program graph.
// If a semantic error occurs (e.g. type error),
// and error message is output, execution stops,
// and the function returns.
//

//
// Helper to execute a function‐call statement.
// Returns true on success, false on semantic error.
// Private to execute.c:
//
//
// Helper to execute a function‐call statement.
// Returns true if executed successfully, false on semantic error.
// Private to execute.c:
//


//
// execute.c (add this above execute()):
//
//helper used to execute function calls
static bool
execute_function_call(struct STMT *stmt, struct RAM *memory)
{
    const char     *fname = stmt->types.function_call->function_name;
    struct ELEMENT *param = stmt->types.function_call->parameter;

    /* only support print() */
    if (strcmp(fname, "print") != 0) {
        printf("**SEMANTIC ERROR: unknown function '%s' (line %d)\n",
               fname, stmt->line);
        return false;
    }

    /* no-arg print() → newline */
    if (param == NULL) {
        putchar('\n');
        return true;
    }

    /* string literal */
    if (param->element_type == ELEMENT_STR_LITERAL) {
        printf("%s\n", param->element_value);
        return true;
    }

    /* integer literal */
    if (param->element_type == ELEMENT_INT_LITERAL) {
        int iv = atoi(param->element_value);
        printf("%d\n", iv);
        return true;
    }

    /* real literal */
    if (param->element_type == ELEMENT_REAL_LITERAL) {
        double dv = atof(param->element_value);
        printf("%lf\n", dv);
        return true;
    }

    /* identifier → variable lookup */
    if (param->element_type == ELEMENT_IDENTIFIER) {
        struct RAM_VALUE *rvp = ram_read_cell_by_name(memory,
                                                     param->element_value);
        /* NEW: check for undefined variable */
        if (rvp == NULL) {
            printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n",
                   param->element_value, stmt->line);
            return false;
        }
        switch (rvp->value_type) {
          case RAM_TYPE_INT:
            printf("%d\n", rvp->types.i);
            break;
          case RAM_TYPE_REAL:
            printf("%lf\n", rvp->types.d);
            break;
          case RAM_TYPE_STR:
            printf("%s\n", rvp->types.s);
            break;
          default:
            printf("**SEMANTIC ERROR: cannot print value (line %d)\n",
                   stmt->line);
            ram_free_value(rvp);
            return false;
        }
        ram_free_value(rvp);
        return true;
    }

    /* anything else → error */
    printf("**SEMANTIC ERROR: invalid print argument (line %d)\n",
           stmt->line);
    return false;
}

//
// 
//
//
// Helper to execute an assignment statement of the form:
//   variable = <int | real | string literal>
// Returns true on success, false on semantic error.
// Private to execute.c:
//
//
// 
//

// Evaluates a binary integer expression (literals or variables) with operators
// +, -, *, /, %, **.  Returns true on success (stores the result in *result),
// or false on semantic error (and prints an error).
//


static bool
execute_binary_expression(struct EXPR *expr,
                          struct RAM  *memory,
                          int           line_no,
                          int         *result)
{
    int left, right;

    // --- Fetch left operand ---
    struct UNARY_EXPR *l_un = expr->lhs;
    struct ELEMENT    *l_elt = l_un->element;
    if (l_elt->element_type == ELEMENT_INT_LITERAL) {
        left = atoi(l_elt->element_value);
    }
    else if (l_elt->element_type == ELEMENT_IDENTIFIER) {
        struct RAM_VALUE *rv = ram_read_cell_by_name(memory,
                                                     l_elt->element_value);
        if (!rv) {
            printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n",
                   l_elt->element_value, line_no);
            return false;
        }
        if (rv->value_type != RAM_TYPE_INT) {
            printf("**SEMANTIC ERROR: expected integer (line %d)\n", line_no);
            ram_free_value(rv);
            return false;
        }
        left = rv->types.i;
        ram_free_value(rv);
    }
    else {
        printf("**SEMANTIC ERROR: expected integer literal or variable (line %d)\n",
               line_no);
        return false;
    }

    // --- Fetch right operand ---
    struct UNARY_EXPR *r_un = expr->rhs;
    struct ELEMENT    *r_elt = r_un->element;
    if (r_elt->element_type == ELEMENT_INT_LITERAL) {
        right = atoi(r_elt->element_value);
    }
    else if (r_elt->element_type == ELEMENT_IDENTIFIER) {
        struct RAM_VALUE *rv = ram_read_cell_by_name(memory,
                                                     r_elt->element_value);
        if (!rv) {
            printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n",
                   r_elt->element_value, line_no);
            return false;
        }
        if (rv->value_type != RAM_TYPE_INT) {
            printf("**SEMANTIC ERROR: expected integer (line %d)\n", line_no);
            ram_free_value(rv);
            return false;
        }
        right = rv->types.i;
        ram_free_value(rv);
    }
    else {
        printf("**SEMANTIC ERROR: expected integer literal or variable (line %d)\n",
               line_no);
        return false;
    }

    // --- Perform operation ---
    switch (expr->operator_type) {
      case OPERATOR_PLUS:
        *result = left + right;
        return true;
      case OPERATOR_MINUS:
        *result = left - right;
        return true;
      case OPERATOR_ASTERISK:
        *result = left * right;
        return true;
      case OPERATOR_DIV:
        if (right == 0) {
          printf("**SEMANTIC ERROR: divide by 0 (line %d)\n", line_no);
          return false;
        }
        *result = left / right;
        return true;
      case OPERATOR_MOD:
        if (right == 0) {
          printf("**SEMANTIC ERROR: mod by 0 (line %d)\n", line_no);
          return false;
        }
        *result = left % right;
        return true;
      case OPERATOR_POWER:
        {
          int acc = 1;
          for (int i = 0; i < right; i++)
            acc *= left;
          *result = acc;
          return true;
        }
      default:
        printf("**SEMANTIC ERROR: unsupported operator (line %d)\n", line_no);
        return false;
    }
}

//most important function helps execute assignment statments
static bool
execute_assignment(struct STMT *stmt, struct RAM *memory)
{
    const char   *var_name = stmt->types.assignment->var_name;
    struct VALUE *rhs_val  = stmt->types.assignment->rhs;

    /* Validate RHS is an expression */
    if (!rhs_val || rhs_val->value_type != VALUE_EXPR) {
        printf("**SEMANTIC ERROR: invalid RHS in assignment (line %d)\n",
               stmt->line);
        return false;
    }
    struct EXPR *expr = rhs_val->types.expr;

    /* 1) Pointer‐based assignment? */
    if (stmt->types.assignment->isPtrDeref) {
        /* Read the pointer cell’s value; ptrs are stored as ints */
        struct RAM_VALUE *pval = ram_read_cell_by_name(memory,
                                                       (char*)var_name);
        if (!pval) {
            printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n",
                   var_name, stmt->line);
            return false;
        }
        /* Accept either an INT or a PTR tag as a valid address holder */
        if (pval->value_type != RAM_TYPE_INT &&
            pval->value_type != RAM_TYPE_PTR) {
            printf("**SEMANTIC ERROR: invalid pointer value (line %d)\n",
                   stmt->line);
            ram_free_value(pval);
            return false;
        }
        int addr = pval->types.i;
        ram_free_value(pval);

        /* Evaluate RHS into rv */
        struct RAM_VALUE rv;
        if (expr->isBinaryExpr) {
            int computed;
            if (!execute_binary_expression(expr, memory, stmt->line,
                                           &computed))
                return false;
            rv.value_type = RAM_TYPE_INT;
            rv.types.i    = computed;
        }
        else {
            struct UNARY_EXPR *unary = expr->lhs;
            struct ELEMENT    *elt   = unary->element;
            switch (elt->element_type) {
              case ELEMENT_INT_LITERAL:
                rv.value_type = RAM_TYPE_INT;
                rv.types.i    = atoi(elt->element_value);
                break;
              case ELEMENT_REAL_LITERAL:
                rv.value_type = RAM_TYPE_REAL;
                rv.types.d    = atof(elt->element_value);
                break;
              case ELEMENT_STR_LITERAL:
                rv.value_type = RAM_TYPE_STR;
                rv.types.s    = elt->element_value;
                break;
              case ELEMENT_IDENTIFIER:
              {
                struct RAM_VALUE *src = ram_read_cell_by_name(
                   memory, elt->element_value);
                if (!src) {
                    printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n",
                           elt->element_value, stmt->line);
                    return false;
                }
                rv = *src;  // copy entire value (including string pointer)
                ram_free_value(src);
                break;
              }
              default:
                printf("**SEMANTIC ERROR: expected literal or variable (line %d)\n",
                       stmt->line);
                return false;
            }
        }

        /* Write by address */
        if (!ram_write_cell_by_addr(memory, rv, addr)) {
            printf("**SEMANTIC ERROR: invalid memory address for assignment (line %d)\n",
                   stmt->line);
            return false;
        }
        return true;
    }

    /* 2) Non-pointer assignment */

    /* Binary-expression? */
    if (expr->isBinaryExpr) {
        int computed;
        if (!execute_binary_expression(expr, memory, stmt->line,
                                       &computed))
            return false;
        struct RAM_VALUE rv;
        rv.value_type = RAM_TYPE_INT;
        rv.types.i    = computed;
        ram_write_cell_by_name(memory, rv, (char*)var_name);
        return true;
    }

    /* Single-term (literal or variable) */
    struct UNARY_EXPR *unary = expr->lhs;
    struct ELEMENT    *elt   = unary->element;

    /* Identifier: fetch & write immediately */
    if (elt->element_type == ELEMENT_IDENTIFIER) {
        struct RAM_VALUE *src = ram_read_cell_by_name(memory,
                                                     elt->element_value);
        if (!src) {
            printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n",
                   elt->element_value, stmt->line);
            return false;
        }
        ram_write_cell_by_name(memory, *src, (char*)var_name);
        ram_free_value(src);
        return true;
    }

    /* Literal cases */
    struct RAM_VALUE rv;
    switch (elt->element_type) {
      case ELEMENT_INT_LITERAL:
        rv.value_type = RAM_TYPE_INT;
        rv.types.i    = atoi(elt->element_value);
        break;
      case ELEMENT_REAL_LITERAL:
        rv.value_type = RAM_TYPE_REAL;
        rv.types.d    = atof(elt->element_value);
        break;
      case ELEMENT_STR_LITERAL:
        rv.value_type = RAM_TYPE_STR;
        rv.types.s    = elt->element_value;
        break;
      default:
        printf("**SEMANTIC ERROR: expected literal or variable (line %d)\n",
               stmt->line);
        return false;
    }

    ram_write_cell_by_name(memory, rv, (char*)var_name);
    return true;
}
//executes whole statment
void execute(struct STMT* program, struct RAM* memory)
{
    struct STMT* stmt = program;
    while (stmt != NULL) {
        if (stmt->stmt_type == STMT_ASSIGNMENT) {
            /* call our helper, abort on error */
            if (!execute_assignment(stmt, memory)) {
                return;
            }
            stmt = stmt->types.assignment->next_stmt;
        }
        else if (stmt->stmt_type == STMT_FUNCTION_CALL) {
            if (!execute_function_call(stmt, memory)) {
                return;
            }
            stmt = stmt->types.function_call->next_stmt;
        }
        else {
            assert(stmt->stmt_type == STMT_PASS);
            stmt = stmt->types.pass->next_stmt;
        }
    }
}
