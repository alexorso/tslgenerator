

#include <stdio.h>

#include "defines.h"    /* type definitions and macros for flags and MAX limits */
#include "structs.h"    /* structures used (needs defines.h) */
#include "maindefs.h"   /* global variables defined in main.c */

static FILE*    file_ptr;       /* the output file */
static int      num_frames;     /* number of frames generated */
static boolean  count_only;     /* whether we should write frames or only count them */
static short    choice_idx[ MAX_TOT_CATS ];     /* indexes of one choice from each category,
                                                   which all together make up a frame */

/* **** debug **** */
#if DEBUG
int output_vars_size = sizeof( file_ptr ) + sizeof( num_frames ) + sizeof( count_only ) + sizeof( choice_idx );
#endif


/* local functions */
int     generator( Flag flags );

void    make_singles( void );
void    write_single( Category* the_cat, Choice* the_choice, char* the_single, char* if_or_else );

void    make_frames( short depth );
void    write_frame( void );
void    set_alt_props( Choice* curr_choice, Property* alt_props[], short* num_alt_props, Flag flag );
boolean eval_expr( Expression* expr );


/* Generate the frames according to what flags are set */
int generator( Flag flags )
{
    count_only = flags & COUNT_ONLY;
    num_frames = 0;
    
    if ( !count_only )
    {
        if ( flags & STD_OUTPUT )
            file_ptr = stdout;
        else
            file_ptr = fopen( out_file, "w" );
    }
    
    /* make_singles (through write_single) and make_frames increment num_frames */
    make_singles();
    make_frames( 0 );
    
    return num_frames;
}


/* Make the single-type frames from each category */
void make_singles( void )
{
    Choice*     curr_choice;
    short       i, j;
    
    for ( i = 0; i < tot_cats; i++ )
        for ( j = 0; j < cats[i] -> num_choices; j++ )
        {
            curr_choice = cats[i] -> choices[j];
           
            if ( curr_choice -> single != NULL )
                write_single( cats[i], curr_choice, curr_choice -> single, NULL );
            else
            {
                if ( curr_choice -> if_single != NULL )
                    write_single( cats[i], curr_choice, curr_choice -> if_single, "if" );
                    
                if ( curr_choice -> else_single != NULL )
                    write_single( cats[i], curr_choice, curr_choice -> else_single, "else" );
            }
        }
}


/* Write a single-type frame */
void write_single( Category* the_cat, Choice* the_choice, char* the_single, char* if_or_else )
{
    num_frames++;
    if ( count_only )
        return;
    
    fprintf( file_ptr, "\nTest Case %-3d\t\t<%s>", num_frames, the_single );

    if ( if_or_else != NULL )
        fprintf( file_ptr, "  (follows [%s])\n", if_or_else );
    else
        fprintf( file_ptr, "\n" );
        
    fprintf( file_ptr, "   %s :  %s\n\n", the_cat -> name, the_choice -> name );
}


/* Recursively make the normal (non-single) frames.
 * Depth indicates which category we're in.
 * make_frames should initially be called with depth = 0
 */
void make_frames( short depth )
{
    Category*   this_cat = cats[ depth ];
    Choice*     curr_choice;
    Property*   alt_props[ MAX_CHOICE_PROPS ];  /* locations of properties altered by this call */
    
    boolean     made_it = FALSE;
    short       num_alt_props,
                i, j;
    
    /* base case:  we've gone through all the categories, so write the frame if necessary */
    if ( depth >= tot_cats )
    {
        num_frames++;
        if ( !count_only )
            write_frame();
        return;
    }
    
    /* try each choice in the current category */
    for ( i = 0; i < this_cat -> num_choices; i++ )
    {
        curr_choice = this_cat -> choices[i];
        num_alt_props = 0;
        
        if ( curr_choice -> single != NULL )
            continue;
        else if ( curr_choice -> flags & IF_EXPR )
        {
            if ( eval_expr( curr_choice -> if_expr ) )
            {
                if ( curr_choice -> if_single != NULL )
                    continue;
                else
                    set_alt_props( curr_choice, alt_props, &num_alt_props, IF_EXPR );
            }
            else if ( curr_choice -> flags & ELSE_EXPR )
            {
                if ( curr_choice -> else_single != NULL )
                    continue;
                else
                    set_alt_props( curr_choice, alt_props, &num_alt_props, ELSE_EXPR );
            }
            else
                continue;
        }
        else
            set_alt_props( curr_choice, alt_props, &num_alt_props, 0 );
        
        made_it = TRUE;             /* we've successfully selected a choice from this category */
        choice_idx[ depth ] = i;    /* store the index of the choice for printing later */
        make_frames( depth + 1 );   /* the recursive call */
        
        /* reset properties altered by set_alt_props */
        for ( j = 0; j < num_alt_props; j++ )
            alt_props[j] -> value = FALSE;
    }
    if ( !made_it )  /* we weren't able to select a choice from this category */
    {
        choice_idx[ depth ] = -1;
        make_frames( depth + 1 );
    }
}


/* Write a normal frame */
void write_frame( void )
{
    short i;
    
    fprintf( file_ptr, "\nTest Case %-3d\t\t(Key = ", num_frames );
    
    for ( i = 0; i < tot_cats; i++ )
    {
        if ( choice_idx[i] >= 0 )
            fprintf( file_ptr, "%d.", choice_idx[i] + 1 );
        else
            fprintf( file_ptr, "0." );
    }
    fprintf( file_ptr, ")\n" );
    
    for ( i = 0; i < tot_cats; i++ )
    {
        fprintf( file_ptr, "   %-*s :  ", max_cat_name, cats[i] -> name );
        if ( choice_idx[i] >= 0 )
            fprintf( file_ptr, "%s\n", cats[i] -> choices[ choice_idx[i] ] -> name );
        else
            fprintf( file_ptr, "<n/a>\n" );
    }
    fprintf( file_ptr, "\n" );
}


/* Set the properties associated with the choice being made to true.
 * We store the addresses of the properties we change in alt_props
 * (and how many in num_alt_props) so we can change them back later.
 */
void set_alt_props( Choice* curr_choice, Property* alt_props[], short* num_alt_props, Flag flag )
{
    short       num_props, i;
    Property**  props;
    
    switch ( flag )
    {
        case 0:
            num_props = curr_choice -> num_props;
            props = curr_choice -> props;
        break;
        case IF_EXPR:
            num_props = curr_choice -> num_if_props;
            props = curr_choice -> if_props;
        break;
        case ELSE_EXPR:
            num_props = curr_choice -> num_else_props;
            props = curr_choice -> else_props;
        break;
    }
    
    for ( i = 0; i < num_props; i++ )
        if ( !( props[i] -> value ) )
        {
            alt_props[ ( *num_alt_props )++ ] = props[i];
            props[i] -> value = TRUE;
        }
}


/* Evaluate an expression represented as a structure */
boolean eval_expr( Expression* expr )
{
    boolean a, b;
    
    if ( expr -> flags & EXPR_A )
        a = eval_expr( expr -> exprA );
    else
        a = expr -> propA -> value;     /* Expression* -> Property* -> short */
    if ( expr -> flags & EXPR_B )
        b = eval_expr( expr -> exprB );
    else
        b = expr -> propB -> value;     /* Expression* -> Property* -> short */

    if ( expr -> flags & NOT_A ) a = !a;
    if ( expr -> flags & NOT_B ) b = !b;
    
    if ( expr -> flags & OP_AND )
        return a && b;
    else
        return a || b;
}
