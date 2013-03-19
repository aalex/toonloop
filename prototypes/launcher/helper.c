/*
 * Compile me with:
 *   gcc -o helper helper.c
 */
#include <stdio.h>
 
int
main( int    argc,
      char **argv )
{
    int i;
 
    for( i = 0; i < 10; i++ )
    {
        char stdout_string[] = "Normal message no:  .";
        char stderr_string[] = "Error message no:  .";
 
        stdout_string[19] = '0' + i;
        stderr_string[18] = '0' + i;
 
        sleep( 1 );
        fprintf( stdout, "%s\n", stdout_string );
        sleep( 1 );
        fprintf( stderr, "%s\n", stderr_string );
    }
 
    return( 0 );
}
