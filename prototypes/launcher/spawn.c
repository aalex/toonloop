/**
 * Compile with:
 * gcc -Wall `pkg-config --libs --cflags glib-2.0` -o spawn spawn.c
 */
#include <glib.h>
//#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static gint std_output;

static void run(void)
{
    GError *err;
    err = NULL;
    gchar *args[] = {
        "sh",
        "-c",
        "echo hello",
        NULL
        };
    GSpawnFlags flags = G_SPAWN_SEARCH_PATH;
    GPid pid;
    if (! g_spawn_async_with_pipes(
        NULL, /* inherit working directory */
        args,
        NULL, /* inherit env vars */
        flags,
        NULL, /* function to call after fork(), before exec() */
        NULL, /* user data */
        &pid,
        NULL,
        &std_output,
        NULL,
        &err))
    {
        g_warning("Error: %s\n", err->message);
        g_error_free(err);
        exit(1);
    }
    else
    {
        g_print("success\n");    
    }
    /** TODO: close std_output socket */
}

int main(int argc, char *argv[])
{
   run();
   return 0;
}

