


#include "defines.h"    /* type definitions and macros for flags and MAX limits */

/* **** debug **** */
#if DEBUG

#include <stdio.h>
#include "structs.h"    /* structures used (needs defines.h) */
#include "maindefs.h"   /* global variables defined in main.c */

extern int main_vars_size, input_vars_size, output_vars_size;


/* local functions */
void debug_info( void );
int  vars_size( void );
void print_expr( Expression* expr );


void debug_info( void )
{
    Choice*     curr_choice;
    short       i, j;
    
    printf( "\n" );
    for ( i = 0; i < tot_cats; i++ )
        for ( j = 0; j < cats[i] -> num_choices; j++ )
        {
            curr_choice = cats[i] -> choices[j];
            if ( curr_choice -> flags & IF_EXPR )
            {
                printf( "%d.%d   [if ", i + 1, j + 1 );
                print_expr( curr_choice -> if_expr );
                printf( "]\n" );
            }
        }
    
    printf( "\nmax number of total categories %d\n", MAX_TOT_CATS );
    printf( "max number of total properties %d\n", MAX_TOT_PROPS );
    printf( "max length of each category name %d\n", MAX_CAT_STR - 1 );
    printf( "max number of choices per category %d\n", MAX_CAT_CHOICES );
    printf( "max length of each choice name %d\n", MAX_CHOICE_STR - 1 );
    printf( "max number of properties per choice %d\n", MAX_CHOICE_PROPS );
    printf( "max length of each property name %d\n", MAX_PROP_STR - 1 );
        
    printf( "\n%d categories, %d properties\n", tot_cats, tot_props );
}


int vars_size( void )
{
    return main_vars_size + input_vars_size + output_vars_size;
}


void print_expr( Expression* expr )
{
    if ( expr -> flags & NOT_A )
        printf( "!" );
    if ( expr -> flags & EXPR_A )
    {
        printf( "(" );
        print_expr( expr -> exprA );
        printf( ")" );
    }
    else
        printf( "%s", expr -> propA -> name );
        
    if ( expr -> flags & OP_AND )
        printf( " && " );
    else
        printf( " || " );
        
    if ( expr -> flags & NOT_B )
        printf( "!" );
    if ( expr -> flags & EXPR_B )
    {
        printf( "(" );
        print_expr( expr -> exprB );
        printf( ")" );
    }
    else
        printf( "%s", expr -> propB -> name );
}


#endif
