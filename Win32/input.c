
#include <stdio.h>
#include <stdlib.h>     /* exit, malloc */
#include <string.h>
#include <ctype.h>      /* isspace */

#include "defines.h"    /* type definitions and macros for flags and MAX limits */
#include "structs.h"    /* structures used (needs defines.h) */
#include "maindefs.h"   /* global variables defined in main.c */

static Property     FALSE_PROP = { "F", 0 };    /* generic false property used to complete expressions */
static char         tmp_str[ MAX_TMP_STR ],     /* temp string storage */
                    tmp_char,                   /* temp char storage */
                    *error_str = "error",
                    *single_str = "single";

/* **** debug **** */
#if DEBUG
int input_vars_size = sizeof( FALSE_PROP ) + sizeof( tmp_str ) + sizeof( char ) * 3;
#endif


/* local functions */
void        parse_file( void );
void        init_choice( Choice* a_choice );
void        init_cat( Category* a_cat );
void        parse_constraint( Choice* curr_choice, char* constraint );
Property*   parse_property( char* name, size_t length, boolean add_prop );
Expression* parse_expr( char* start, char* end );
void        parse_operand( Expression* root, char* start, char* end, boolean operandA );
char*       first_operator( char* pattern, char* start, char* end );
char*       close_parens( char* start, char* end );
void        trim( char** start, char** end );


/* Parse the TSL file (in_file, defined in main.c) into categories, choices, etc. */
void parse_file( void )
{
    FILE*       file_ptr = fopen( in_file, "r" );   /* location of FILE for reading */
    Category*   curr_cat = NULL;                    /* location of current working Category */
    Choice*     curr_choice;                        /* location of current working Choice */
    
    if ( file_ptr == NULL )
    {
        printf( "\nThe file '%s' could not be opened.\nQuitting\n\n", in_file );
        exit( EXIT_FAILURE );
    }
    
    while ( !feof( file_ptr ) )
    {
        /* skip comments */
        while ( fscanf( file_ptr, " %1[#]", tmp_str ) > 0 )
            fscanf( file_ptr, "%*[^\n]\n" );
        
        /* in case comments are the last thing in the file */
        if ( feof( file_ptr ) )
            break;
        
        if ( fscanf( file_ptr, " %[^:.#\n]%c", tmp_str, &tmp_char ) > 0 )
        {
            switch ( tmp_char )
            {
                case ':':   /* category */
                    /* remove categories that don't have any choices */
                    if ( curr_cat != NULL && curr_cat -> num_choices == 0 )
                    {
                        /* **** debug **** */
                        #if DEBUG
                        printf( "removing cat '%s'\n", curr_cat -> name );
                        #endif

                        free( curr_cat );
                        tot_cats--;
                    }
                    
                    /* **** debug **** */
                    #if DEBUG
                    printf( "adding cat '%s'\n", tmp_str );
                    #endif
        
                    curr_cat = cats[ tot_cats ] = malloc( sizeof( Category ) );
                    
                    init_cat( curr_cat );
                    strncpy( curr_cat -> name, tmp_str, MAX_CAT_STR );
                    if ( strlen( curr_cat -> name ) > max_cat_name )
                        max_cat_name = strlen( curr_cat -> name );
                    tot_cats++;
                break;
                case '.':   /* choice */
                    /* in case there is a choice before any categories */
                    if ( curr_cat == NULL )
                    {
                        printf( "\nThe choice '%s' doesn't belong to a category.\nQuitting\n\n", tmp_str );
                        exit( EXIT_FAILURE );
                    }
                            
                    /* **** debug **** */
                    #if DEBUG
                    printf( "adding choice '%s'\n", tmp_str );
                    #endif
        
                    curr_choice = malloc( sizeof( Choice ) );
                    curr_cat -> choices[ curr_cat -> num_choices ] = curr_choice;
                    
                    init_choice( curr_choice );
                    strncpy( curr_choice -> name, tmp_str, MAX_CHOICE_STR );
                    ( curr_cat -> num_choices )++;
                    
                    while ( fscanf( file_ptr, " [ %[^]#\n] ]", tmp_str ) > 0 )
                        parse_constraint( curr_choice, tmp_str );
                break;
            }
        }
        else
        {
            printf( "\nAn error occurred parsing the file.\nThe last line parsed was '%s'.\nQuitting\n\n", tmp_str );
            exit( EXIT_FAILURE );
        }
    }
    /* in case the last category had no choices */
    if ( curr_cat != NULL && curr_cat -> num_choices == 0 )
    {
        /* **** debug **** */
        #if DEBUG
        printf( "removing cat '%s'\n", curr_cat -> name );
        #endif
        free( curr_cat );
        tot_cats--;
    }
    fclose( file_ptr );
}


/* Initialize a Choice */
void init_choice( Choice* a_choice )
{
    a_choice -> flags = 0;
    a_choice -> num_props = a_choice -> num_if_props = a_choice -> num_else_props = 0;
    a_choice -> single = a_choice -> if_single = a_choice -> else_single = NULL;
}


/* Initialize a Category */
void init_cat( Category* a_cat )
{
    a_cat -> num_choices = 0;
}


/* Parse the constraint and store the information in the current Choice.
 * Possible constraints are single, error, property, if, and else.
 */
void parse_constraint( Choice* curr_choice, char* constraint )
{
    /* we need a local temp string because 'char* constraint' points to
       the global temp string 'tmp_str' */
    char        tmp_cstr[ MAX_TMP_STR ];
    char*       tmp_single;
    Property*   tmp_prop;
    
    sscanf( constraint, "%s", tmp_cstr );
    
    if ( strcmp( tmp_cstr, single_str ) == 0 || strcmp( tmp_cstr, error_str ) == 0 )
    {
        if ( strcmp( tmp_cstr, single_str ) == 0 )
            tmp_single = single_str;
        else
            tmp_single = error_str;
            
        if ( curr_choice -> flags & ELSE_EXPR )
            curr_choice -> else_single = tmp_single;
        else if ( curr_choice -> flags & IF_EXPR )
            curr_choice -> if_single = tmp_single;
        else
            curr_choice -> single = tmp_single;
    }
    else if ( strcmp( tmp_cstr, "property" ) == 0 )
    {
        char*   end = constraint + strlen( constraint );
        short   chars_read;
        
        constraint += 8;  /* move past the word 'property' */
        
        while ( sscanf( constraint, " %[^, ] %hn", tmp_cstr, &chars_read ) > 0 )
        {
            tmp_prop = parse_property( tmp_cstr, strlen( tmp_cstr ), TRUE );
            
            if ( curr_choice -> flags & ELSE_EXPR )
                curr_choice -> else_props[ ( curr_choice -> num_else_props )++ ] = tmp_prop;
            if ( curr_choice -> flags & IF_EXPR )
                curr_choice -> if_props[ ( curr_choice -> num_if_props )++ ] = tmp_prop;
            else
                curr_choice -> props[ ( curr_choice -> num_props )++ ] = tmp_prop;
                
            constraint += chars_read + 1;
            if ( constraint >= end )
                break;
        }
    }
    else if ( strcmp( tmp_cstr, "if" ) == 0 )
    {
        if ( sscanf( constraint, "%*s %[^]]", tmp_cstr ) > 0 )
        {
            curr_choice -> flags = curr_choice -> flags | IF_EXPR;
            curr_choice -> if_expr = parse_expr( tmp_cstr, tmp_cstr + strlen( tmp_cstr ) );
            curr_choice -> if_props = &( curr_choice -> props[ curr_choice -> num_props ] );
        }
    }
    else if ( strcmp( tmp_cstr, "else" ) == 0 )
    {
        curr_choice -> flags = curr_choice -> flags | ELSE_EXPR;
        curr_choice -> else_props = &( curr_choice -> if_props[ curr_choice -> num_if_props ] );
    }
}


/* Parse the property pointed to by name, add it if add_prop is true
 * and it hasn't been added before, and return the address of the Property.
 * 'length' is needed because name points to a position within the global
 * temp string, so the property name we want isn't null-terminated.
 */
Property* parse_property( char* name, size_t length, boolean add_prop )
{
    char    tmp_name[ MAX_PROP_STR ];
    short   i;
    
    if ( length >= MAX_PROP_STR )
        length = MAX_PROP_STR - 1;
    strncpy( tmp_name, name, length );
    tmp_name[ length ] = '\0';
    
    for ( i = 0; i < tot_props; i++ )
        if ( strcmp( props[i] -> name, tmp_name ) == 0 )
            return props[i];
    
    if ( add_prop )
    {
        /* **** debug **** */
        #if DEBUG
        printf( "adding prop '%s'\n", tmp_name );
        #endif
        
        props[ tot_props ] = malloc( sizeof( Property ) );
        strcpy( props[ tot_props ] -> name, tmp_name );
        props[ tot_props ] -> value = FALSE;
        return props[ tot_props++ ];
    }

    printf( "\nThe property '%s' is not defined.\nQuitting\n\n", tmp_name );
    exit( EXIT_FAILURE );
}


/* Parse the expression in the string bounded by 'start' and 'end'
 * into an Expression (structure) and return its address.
 */
Expression* parse_expr( char* start, char* end )
{
    char            *first_or, *first_and;
    char            *endA, *startB;
    Expression*     root = malloc( sizeof( Expression ) );
    
    trim( &start, &end );
    
    first_or = first_operator( " || ", start, end );
    first_and = first_operator( " && ", start, end );
    
    if ( first_or != NULL || first_and != NULL )
    {
        if ( first_or != NULL )
        {
            endA = first_or;
            root -> flags = 0;
        }
        else
        {
            endA = first_and;
            root -> flags = OP_AND;
        }
        startB = endA + 4;
        parse_operand( root, start, endA, TRUE );
        parse_operand( root, startB, end, FALSE );
    }
    else
    {
        root -> flags = 0;
        root -> propB = &FALSE_PROP;
        parse_operand( root, start, end, TRUE );
    }
    return root;
}


/* Parse the operand in the string bounded by 'start' and 'end' and store
 * the info in 'root'.  If 'operandA' is true its the left operand, if not
 * its the right operand.
 */
void parse_operand( Expression* root, char* start, char* end, boolean operandA )
{
    Flag            not_flag, expr_flag;
    Property**      prop;
    Expression**    expr;
    boolean         not = FALSE;
    char*           old_start;
    
    if ( operandA )
    {
        not_flag = NOT_A;
        expr_flag = EXPR_A;
        prop = &( root -> propA );
        expr = &( root -> exprA );
    }
    else
    {
        not_flag = NOT_B;
        expr_flag = EXPR_B;
        prop = &( root -> propB );
        expr = &( root -> exprB );
    }
    
    trim( &start, &end );
    
    if ( *start == '!' )
    {
        not = TRUE;
        old_start = start++;
        trim( &start, &end );
    }
    /* parenthesis matching already checked by parse_expr >> first_operator >> close_parens */
    if ( *start == '(' )
    {
        if ( not )
            root -> flags = root -> flags | not_flag;
        root -> flags = root -> flags | expr_flag;
        *expr = parse_expr( start + 1, end - 1 );  /* recursive call */
    }
    else if ( first_operator( " || ", start, end ) != NULL ||
              first_operator( " && ", start, end ) != NULL )
    {
        if ( not )
            start = old_start;
        root -> flags = root -> flags | expr_flag;
        *expr = parse_expr( start, end );  /* recursive call */
    }
    else
    {
        if ( not )
            root -> flags = root -> flags | not_flag;
        *prop = parse_property( start, (size_t) ( end - start ), FALSE );
    }
}


/* Return the position of the first operator (passed in as 'pattern') not
 * contained within parentheses in the string bounded by 'start' and 'end'.
 */
char* first_operator( char* pattern, char* start, char* end )
{
    char *position, *open, *close;
    
    close = start;
    
    while ( ( position = strstr( close, pattern ) ) != NULL && position < end )
    {
        while ( position > close )
        {
            open = strchr( close, '(' );
            if ( open != NULL && open < end )
                close = close_parens( open, end );
            else
                return position;
        }
        if ( position > open && position < close )
            continue;
        else
            return position;
    }
    
    return NULL;
}


/* Return the position of the closing parenthesis.
 * 'start' points at the opening parenthesis.
 * 'end' points at the end of the string.
 */
char* close_parens( char* start, char* end )
{
    char*   position = start + 1;
    short   stack = 0;
    
    for( ; position < end; position++ )
    {
        if ( *position == ')' )
        {
            if ( stack == 0 )
                return position;
            else
                stack--;
        }
        else if ( *position == '(' )
            stack++;
    }
    
    printf( "\nThe expression '%.*s' has an unmatched parenthesis.\nQuitting\n\n", end - start, start );
    exit( EXIT_FAILURE );
}


/* Trim whitespace from both ends of a string bounded by '*start' and '*end' */
void trim( char** start, char** end )
{
    for ( ; *start < *end; *start++ )
        if ( !isspace( (int) **start ) )
            break;
    for ( ; *start < *end; *end-- )
        if ( !isspace( (int) *( *end - 1 ) ) )
            break;
}
