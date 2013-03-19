#include <iostream>
#include <glib.h>

bool run_toonloop()
{
    gchar *working_directory = ".";
    gchar **argv = { "toonloop", "-v", NULL };
    gchar **envp = { NULL };
    GSpawnFlags flags = G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD;
    GSpawnChildSetupFunc child_setup = NULL;
    gpointer user_data = NULL;
    GPid child_pid;
    gint standard_input;
    gint standard_output;
    gint standard_error;
    GError *error = NULL;

    gboolean success = g_spawn_async_with_pipes(
        working_directory, argv, envp, flags,
        child_setup, user_data,
        &child_pid,
        &standard_input, &standard_output, &standard_error,
        &error); 

    if (success)
    {
        std::cout << "success!" << std::endl;
    }
    else
    {
        std::cout << "Error: " << error->message << std::endl;
        g_error_free(error);
        return false;
    }
    return true;
}


int main(int argc, char *argv[])
{
    GMainLoop *main_loop = NULL;
    main_loop = g_main_loop_new(NULL, FALSE);
    g_type_init();
    g_main_loop_run(main_loop);
    return 0;
}

