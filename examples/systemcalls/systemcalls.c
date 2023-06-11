#include "systemcalls.h"
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    int status = system(cmd);
    if (status == -1) {
        return false;
    } else {
        bool success = WIFEXITED(status) && (WEXITSTATUS(status) == 0);
        return success;
    }
}

bool has_absolute_path(const char *command) {
    // Check if the command contains a forward slash
    return strchr(command, '/') != NULL;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

    bool do_exec(int count, ...)
    {
        va_list args;
        va_start(args, count);
        char * command[count+1];
        int i;
        for(i=0; i<count; i++)
        {
            command[i] = va_arg(args, char *);
        }
        command[count] = NULL;
        // this line is to avoid a compile warning before your implementation is complete
        // and may be removed

        if(!has_absolute_path(command[count-1])) {
            return false;
        }

        if (!has_absolute_path(command[0]))
        {
            printf("Command '%s' must be specified with an absolute path or PATH expansion\n", command[0]);
            va_end(args);
            return false;
        }

    /*
    * TODO:
    *   Execute a system command by calling fork, execv(),
    *   and wait instead of system (see LSP page 161).
    *   Use the command[0] as the full path to the command to execute
    *   (first argument to execv), and use the remaining arguments
    *   as second argument to the execv() command.
    *
    */
        // Fork a child process
        pid_t child_pid = fork();

        // If fork failed, print an error to stderr and clean up the variable argument list
        if (child_pid == -1) {
            perror("fork");
            va_end(args);
            return false;
        // If fork created successfully, execute command[0]. If it returns, error occured
        } else if (child_pid == 0) {
            execv(command[0], command);
            perror("execv");
            va_end(args);
            exit(EXIT_FAILURE);
        // In parent process, if waitpid failed, print an error and clean up variable argument list
        } else {
            int status;
            if (waitpid(child_pid, &status, 0) == -1) {
                perror("waitpid");
                va_end(args);
                return false;
            } else {
                // If child process exited normally
                if (WIFEXITED(status)) {
                    int exit_status = WEXITSTATUS(status);
                    printf("Child process existed with status %d\n", exit_status);
                // If child process exited with a signal
                } else if (WIFSIGNALED(status)) {
                    int signal_number = WTERMSIG(status);
                    printf("Child process terminated by signal %d\n", signal_number);
                }
            }
        }

        va_end(args);

        return true;
    }

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];
    printf("command[count-1]: %s\n", command[count-1]);

    if (!has_absolute_path(command[0]))
    {
        printf("Command '%s' must be specified with an absolute path or PATH expansion\n", command[0]);
        va_end(args);
        return false;
    }

/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
    // Fork a child process
    pid_t child_pid = fork();

    // If fork failed, print an error to stderr and clean up the variable argument list
    if (child_pid == -1) {
        perror("fork");
        va_end(args);
        return false;
    // If fork created successfully, execute command[0]. If it returns, error occured
    } else if (child_pid == 0) {

        int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("open");
            va_end(args);
            exit(EXIT_FAILURE);
        }

        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            close(fd);
            va_end(args);
            exit(EXIT_FAILURE);
        }
        close(fd);
        execv(command[0], command);
        perror("execv");
        va_end(args);
        exit(EXIT_FAILURE);
    // In parent process, if waitpid failed, print an error and clean up variable argument list
    } else {
        int status;
        if (waitpid(child_pid, &status, 0) == -1) {
            perror("waitpid");
            va_end(args);
            return false;
        } else {
            // If child process exited normally
            if (WIFEXITED(status)) {
                int exit_status = WEXITSTATUS(status);
                printf("Child process existed with status %d\n", exit_status);
            // If child process exited with a signal
            } else if (WIFSIGNALED(status)) {
                int signal_number = WTERMSIG(status);
                printf("Child process terminated by signal %d\n", signal_number);
            }
        }
    }

    va_end(args);

    return true;
}
