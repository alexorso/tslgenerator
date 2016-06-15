

#include <stdio.h>
#include <stdlib.h>     /* exit, free */
#include <string.h>     /* for manipulating filename */

#include "defines.h"    /* type definitions and macros for flags and MAX limits */
#include "structs.h"    /* structures used (needs defines.h) */
           
           
Category*   cats[ MAX_TOT_CATS ];       /* array of all Categories */
Property*   props[ MAX_TOT_PROPS ];     /* array of all Properties */
short       tot_cats = 0,               /* total Categories */
            tot_props = 0,              /* total Properties */
            max_cat_name = 0;           /* used to format the output to look nice */
char        *in_file = NULL,            /* name of file for input */
            *out_file = NULL;           /* name of file for ouput (if any) */

/* **** debug **** */
#if DEBUG
int main_vars_size = sizeof( cats ) + sizeof( props ) + sizeof( short ) * 3 + sizeof( char ) * 2;
#endif


/* local functions */
Flag parse_args( int num_args, char* args[] );
void print_man( char* prog_name );
int  cleanup( void );
int  free_expr( Expression* expr );

/* input.c functions */
void parse_file( void );

/* output.c functions */
int generator( Flag flags );

/* **** debug **** */
#if DEBUG
void debug_info( void );
int  vars_size( void );
#endif


int main( int argc, char* argv[] )
{
    Flag    flags;           /* program flags */
    int     num_frames;
    char    *filename = NULL,
            answer[ 5 ];     /* user response */

    printf( "\n----------------------------------------");
    printf( "\n  TSLgenerator\n");
    printf( "\n  (C) University of California Irvine,");
    printf( "\n      Oregon State University, and");
    printf( "\n      Georgia Institute of Technology, 2001-2014");
    printf( "\n----------------------------------------\n");

    if ( argc == 1 )
    {
        printf( "\nUSAGE:  %s [ --manpage ] [ -cs ] input_file [ -o output_file ]\n\n", argv[0] );
        return EXIT_SUCCESS;
    }
    else
        flags = parse_args( argc, argv );
        
    if ( in_file == NULL )
    {
        printf( "\nNo input file provided.\nQuitting\n\n" );
        return EXIT_FAILURE;
    }
    
    
    if ( flags & STD_OUTPUT )
        out_file = "the standard output";
    else if ( flags & OUTPUT_FILE )
    {
        if ( out_file == NULL )
        {
            printf( "\nNo output file provided.\nQuitting\n\n" );
            return EXIT_FAILURE;
        }
    }
    else
    {
        filename = malloc(sizeof(char) * strlen(in_file) + 4);
        strcpy( filename, in_file );
        strcat( filename, ".tsl" );
        out_file = filename;
        free(filename);
    }
    parse_file();

    /* **** debug **** */
    #if DEBUG
    debug_info();
    #endif
    
    num_frames = generator( flags );
    
    if ( flags & COUNT_ONLY )
    {
        printf( "\n\t%d test frames generated\n\n", num_frames );
        printf( "Write test frames to %s (y/n)? ", out_file );
        scanf( "%s", answer );
        if ( answer[0] == 'y' || answer[0] == 'Y' )
            printf( "\n\t%d test frames written to %s\n\n", generator( flags & ~COUNT_ONLY ), out_file );
    }
    else
        printf( "\n\t%d test frames generated and written to %s\n\n", num_frames, out_file );
    
    /* **** debug **** */
    #if DEBUG
    printf( "program base storage  = %d bytes\n", vars_size() );
    printf( "program total storage = %d bytes\n\n", cleanup() + vars_size() );
    #else
    cleanup();
    #endif
    
    return EXIT_SUCCESS;
}


/* Parse the command line arguments and set flags accordingly */
Flag parse_args( int num_args, char* args[] )
{
    Flag    flags = 0;
    short   i, j;
    
    for ( i = 1; i < num_args; i++ )
    {
        if ( strcmp( args[i], "--manpage" ) == 0 )
        {
            print_man( args[0] );
            exit( EXIT_SUCCESS );
        }
        
        if ( *args[i] == '-' )
            for ( j = 1; j < strlen( args[i] ); j++ )
            {
                switch ( args[i][j] )
                {
                    case 'c':
                        flags = flags | COUNT_ONLY;
                    break;
                    case 's':
                        flags = flags | STD_OUTPUT;
                    break;
                    case 'o':
                        if ( !( flags & STD_OUTPUT ) )
                        {
                            flags = flags | OUTPUT_FILE;
                            out_file = args[ i + 1 ];
                        }
                        return flags;
                }
            }
        else {
		in_file = args[i];
	}
    }
    return flags;
}


/* Print the tsl manpage */
void print_man( char* prog_name )
{
    printf( "\nNAME\n\ttsl - generate test frames from a specification file\n" );
    printf( "\nSYNOPSIS\n\ttsl [ --manpage ] [ -cs ] input_file [ -o output_file ]\n" );
    printf( "\nDESCRIPTION\n\tThe TSL utility generates test frames from a specification file\n" );
    printf( "\twritten in the extended Test Specification Language.  By default\n" );
    printf( "\tit writes the test frames to a new file created by appending a\n" );
    printf( "\t'.tsl' extension to the input_file's name.  Options can be used\n" );
    printf( "\tto modify the output.\n" );
    printf( "\nOPTIONS\n\tThe following options are supported:\n" );
    printf( "\n\t--manpage\n\t\tPrint this man page.\n" );
    printf( "\n\t-c\tReport the number of test frames generated, but don't\n" );
    printf( "\t\twrite them to the output. After the number of frames is\n" );
    printf( "\t\treported you will be given the option of writing them\n" );
    printf( "\t\tto the output.\n" );
    printf( "\n\t-s\tOutput is the standard output.\n" );
    printf( "\n\t-o output_file\n\t\tOutput is the file output_file unless the -s option is used.\n\n" );
}


/* Free the memory allocated by the program and return how much */
int cleanup( void )
{
    Choice*     curr_choice;
    int         total_size = 0;
    short       i, j;

    for ( i = 0; i < tot_cats; i++ )
    {
        total_size += sizeof( Category );
        for ( j = 0; j < cats[i] -> num_choices; j++ )
        {
            total_size += sizeof( Choice );
            curr_choice = cats[i] -> choices[j];
            
            if ( curr_choice -> flags & IF_EXPR )
                total_size += free_expr( curr_choice -> if_expr );
                
            free( curr_choice );
        }
        free( cats[i] );
    }
    for ( i = 0; i < tot_props; i++ )
    {
        total_size += sizeof( Property );
        free( props[i] );
    }
    return total_size;
}


/* Free all the memory associated with an Expression (recursive) and return how much */
int free_expr( Expression* expr )
{
    int expr_size = sizeof( Expression );
    
    if ( expr -> flags & EXPR_A )
        expr_size += free_expr( expr -> exprA );
    if ( expr -> flags & EXPR_B )
        expr_size += free_expr( expr -> exprB );
        
    free( expr );
    return expr_size;
}
