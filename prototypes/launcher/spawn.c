/**
 * Compile with:
 * gcc -Wall `pkg-config --libs --cflags glib-2.0` -o spawn spawn.c
 */
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void run(void)
{
    GError *err;
    gchar *output = NULL;
    err = NULL;
    if (! g_spawn_command_line_sync(
          "/bin/sh -c 'echo hello'",
          &output, NULL, NULL, &err))
    {
        fprintf(stderr, "Error: %s\n", err->message);
        g_error_free(err);
        exit(1);
    }
    else
    {
        g_assert(output != NULL);
        if (strcmp(output, "hello\n") != 0)
        {
            printf("output was '%s', should have been 'hello'\n", output);
            exit(1);
        }
        else
        {
            printf("output was '%s': good\n", output);
        }
        g_free(output);
    }
}

int main(int argc, char *argv[])
{
   run();
   return 0;
}

