/**
 * Launches a process with glib
 *
 * g++ $(pkg-config --cflags --libs glib-2.0) -lboost_filesystem-mt -o spawn spawn.cpp
 */
#include <stdio.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gutils.h>
#include <stdlib.h> // for symlink
#include <sys/types.h>
#include <sys/stat.h> // we actually use g_lstat
#include <iostream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

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
void test()
{
    int ret;
    struct stat sb;
    int success;

    //TODO in that order:
    // create a directory in /tmp:
    // TODO: std::string name = timing::get_iso_datetime_for_now();
    std::string new_dir_name = std::string(g_build_path("/", g_get_tmp_dir(), "toonloop-temp", NULL));
    // TODO: use g_mkdir ?
    success = g_mkdir(new_dir_name.c_str(), 0700); 
    if (success == 0)
        std::cout << "Successfully created directory " << new_dir_name << std::endl;
    else
        std::cout << "Did not create directory " << new_dir_name << std::endl;
   
    // Create a symbolic link for each image file in the clip.
    std::string old_name = std::string(g_build_path("/", g_get_home_dir(), ".bashrc", NULL));
    std::string new_name = std::string(g_build_path("/", new_dir_name.c_str(), "foo", NULL));
    success = symlink(old_name.c_str(), new_name.c_str());
    if (success == 0)
        std::cout << "Successfully created symlink " << new_name << std::endl;
    else
        std::cout << "Did not create symlink " << new_name << std::endl;
    struct stat stat_buffer;
    ret = g_lstat(new_name.c_str(), &stat_buffer);
    if (ret < 0)
        std::cerr << "Error calling lstat" << std::endl;
    if ((stat_buffer.st_mode & S_IFLNK) != S_IFLNK)
        std::cerr << "The file " << new_name << " is not a symlink" << std::endl;
    
    run_command(std::string("ls -lr ") + new_dir_name);
   
    // Remove the directory and all its children:
    fs::remove_all(new_dir_name);
    if (fs::exists(new_dir_name))
        std::cout << "Could not remove directory " << new_dir_name << std::endl;
    else
        std::cout << "Successfully removed directory " << new_dir_name << std::endl;

}

int main(int argc, char **argv)
{
    run_command("ls -l");
    std::cout << "The next command should fail:" << std::endl;
    run_command("zxclkja sdlkjas d");
    std::cout << "Now trying to mkdir, ln -s, ls -l and rm -rf:" << std::endl;
    test();
    return 0;
}

