/**
 * Launches a process with glib
 *
 * g++ $(pkg-config --cflags --libs glib-2.0) -o spawn spawn.cpp
 */
#include <stdio.h>
#include <glib.h>
#include <stdlib.h> // for symlink
#include <sys/types.h>
#include <sys/stat.h> // we actually use g_lstat
#include <iostream>
/**
 * Runs a command synchronously
 */
bool run_command(std::string command)
{
    GError *error = NULL;
    gint status = 0;
    gchar *stdout_message = NULL;
    gchar *stderr_message = NULL;
    gboolean ret = false;
    
    ret = g_spawn_command_line_sync(command.c_str(), &stdout_message, &stderr_message, &status, &error);
    if ((error && (0 != error->code)) || ! WIFEXITED(status) || WEXITSTATUS(status)) {
        g_warning("Failed to execute command \"%s\", exited: %i, status: %i, stderr: %s, stdout: %s\n", command.c_str(), WIFEXITED(status), WEXITSTATUS(status), stderr_message ? : "", stdout_message ? : "");
    } else {
        g_print("Successfully ran command \"%s\"\n", command.c_str());
        std::cout << "Its output is : " << stdout_message << std::endl;
        ret = TRUE;
    }
    if (error)
        g_error_free (error);
    g_free(stdout_message);
    g_free(stderr_message);
    return ret == TRUE;
}
#if 0
void test()
{
    int ret;
    struct stat sb;

    //TODO in that order:
    // create a directory in /tmp:
    // TODO: std::string name = timing::get_iso_datetime_for_now();
    std::string new_dir_name = std::string(g_build_path("/", g_get_tmp_dir(), "toonloop-temp", NULL));
    int success = g_mkdir(new_dir_name.c_str(), 0x700);
   
    // Create a symbolic link for each image file in the clip.
    std::string old_name = std::string(g_build_path("/", g_get_home_dir(), ".bashrc", NULL));
    std::string new_name = std::string(g_build_path("/", new_dir_name, "foo", NULL));
    int symlink(old_name.c_str(), new_name.c_str())
    struct stat stat_buffer;
    int ret = g_lstat(new_name.c_str(), &stat_buffer);
    if (ret < 0)
        std::cerr << "Error calling lstat" << std::endl;
    if ((stat_buffer.st_mode & S_IFLNK) != S_IFLNK)
        std::cerr << "The file " << new_name << " is not a symlink" << std::endl;
    
    run_command(std::string("ls -l") + new_dir_name);
   
    // Remove the directory and all its children:
    int removed = g_unlink(tmp_dir);
    if (remove == 0) {
        g_print("Removed the directory with all its children.\n");
    } else { 
        g_print("Error removing the directory with all its children.\n");
    } 
}
#endif 

int main(int argc, char **argv)
{
    run_command("ls -l");
    run_command("zxclkja sdlkjas d");
    //test();
    return 0;
}

