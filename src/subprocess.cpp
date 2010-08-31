/*
 * Toonloop
 *
 * Copyright 2010 Alexandre Quessy
 * <alexandre@quessy.net>
 * http://www.toonloop.com
 *
 * Toonloop is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Toonloop is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the gnu general public license
 * along with Toonloop.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <cstdlib>
#include <iostream>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gutils.h>
#include <string>
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
        //std::cout << "Its output is : " << stdout_message << std::endl;
        ret = true;
    }
    if (error)
        g_error_free (error);
    g_free(stdout_message);
    g_free(stderr_message);
    return ret == true;
}


// Old implementation:
#if 0
#include <unistd.h> // environ, usleep, execv
#include <sys/types.h> // pid_t
#include <sys/wait.h> // wait
/**
 * Runs a bash command.
 * Returns false if could not execute it.
 * @rettype: bool
 */
bool run_command(std::string command)
{
    pid_t fork_pid;
    int exec_ret;
    int status;
    int newpid;
    int retval;

    fork_pid = fork();
    if (fork_pid == -1)
    {
        std::cout << "Could not launch the child process." << std::endl;
        return false;
    } else {
        if (fork_pid == 0)
        {
            std::cout << "We are the child process." << std::endl;
            std::cout << "Calling $ bash " << command << std::endl;
            exec_ret = execl("/bin/bash", "/bin/bash", "-c", command.c_str(), NULL);
            if (exec_ret == -1)
            {
                std::cout << "Execution failed." << std::endl;
            } else {
                std::cout << "The child process is done." << std::endl;
            }
        } else {
            std::cout << "We are the parent process." << std::endl;
            while (1) 
            {
                newpid = waitpid(fork_pid, &status, WNOHANG);
                if (newpid == -1) 
                {
                    //std::cout << "An error occured." << std::endl;    
                    if (WIFEXITED(status))
                    {
                        retval = WEXITSTATUS(status);
                        std::cout << "The child exited normally with return value " << retval << std::endl;    
                    } else if (WIFSIGNALED(status)) {
                        std::cout << "The child process exited because of a signal which was not caught." << std::endl;
                    }
                    break;
                } else if (newpid == 0) {
                    std::cout << "No child process with that pid is available." << std::endl;    
                    // TODO: check the value of errno
                    //break;
                } else {
                    std::cout << "The child exited." << std::endl;    
                }
                usleep(50 * 1000); // 50 ms
            }    
            //if (newpid < 0)
            //{
            //    std::cout << "Child process did not return." << std::endl;
            //} else if (newpid == 0) {
            //    std::cout << "State of the child process is not available." << std::endl;
            //    //std::cout << "Child process returned." << std::endl;
            //}
        }
    }
    return true;
}
#endif

