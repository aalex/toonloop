#include <iostream>
#include <unistd.h> // environ, usleep, execv
#include <stdlib.h> // EXIT_SUCCESS
#include <sys/types.h> // pid_t
#include <sys/wait.h> // wait
#include <string>

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


//int main(int argc, char *argv[])
//{
//    std::string command("echo 'Hello'; sleep 1; echo 'bye'");
//    if (run_command(command))
//    {
//        return EXIT_SUCCESS;
//    } else {
//        return EXIT_FAILURE;
//    }
//}
