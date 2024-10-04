#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_LINE_LENGTH 4096
#define PROMPT "penn-shredder# "
#define CATCHPHRASE "Bwahaha ... Tonight, I dine on turtle soup!\n"

int executionTimeout = 0; // Stores the execution timeout value.
volatile sig_atomic_t sigint_sent = 0; // Flag to track SIGINT signal.

// Signal handler function for SIGINT and SIGALRM signals.
void handle_sig(int sig) {
    if (sig == SIGINT) {
        // Handle SIGINT signal: Print a newline and the shell prompt.
        write(STDOUT_FILENO, "\n" PROMPT, strlen(PROMPT));
        sigint_sent = 0; // Reset the flag
    } else if (sig == SIGALRM) {
        // Handle SIGALRM signal: Print the catchphrase.
        write(STDOUT_FILENO, CATCHPHRASE, strlen(CATCHPHRASE));
    }
}

int main(int argc, const char* argv[]) {
    // Parse command-line arguments for the execution timeout.
    if (argc == 2) {
        int timeout = atoi(argv[1]);
        if (timeout < 0) {
            // Handle invalid timeout value: Print an error message.
            char errmsg[] = "Invalid: Negative timeout value!\n";
            write(STDOUT_FILENO, errmsg, strlen(errmsg));
            return EXIT_FAILURE;
        }
        executionTimeout = timeout;
    } else if (argc > 2 || (argc == 2 && atoi(argv[1]) <= 0)) {
        // Handle too many arguments or an invalid timeout value: Print an error message.
        char errmsg[] = "Invalid: Too many arguments or invalid timeout value!\n";
        write(STDOUT_FILENO, errmsg, strlen(errmsg));
        return EXIT_FAILURE;
    }

    // Register signal handlers for SIGINT and SIGALRM.
    signal(SIGINT, handle_sig);
    signal(SIGALRM, handle_sig); // Register SIGALRM handler.

    char cmd[MAX_LINE_LENGTH]; // Buffer to store user commands.
    write(STDOUT_FILENO, PROMPT, strlen(PROMPT)); // Print the initial shell prompt.

    while (1) {
        char c;
        int i = 0; // Index for reading characters into the cmd buffer.

        while (1) {
            // Read characters from the standard input one at a time.
            if (read(STDIN_FILENO, &c, 1) <= 0) {
                // Handle end of input: Print a newline and exit the loop.
                write(STDOUT_FILENO, "\n", 1);
                exit(EXIT_SUCCESS); // Exit the shell on end of input.
            }

            if (c == '\n') {
                // Handle user pressing Enter: Null-terminate the cmd buffer and reset the index.
                cmd[i] = '\0';
                i = 0;

                if (cmd[0] == '\0') {
                    // Reprompt if the user just presses Enter without entering a command.
                    write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
                    continue;
                }

                // Handle execution of user command here (implement fork, execve, and wait).

                pid_t child_pid = fork(); // Create a child process.
                if (child_pid == -1) {
                    // Handle fork failure: Print an error message and exit with failure status.
                    char errmsg[] = "Fork failed.\n";
                    write(STDOUT_FILENO, errmsg, strlen(errmsg));
                    exit(EXIT_FAILURE);
                }

                if (child_pid == 0) { // Child process
                    char* argv[MAX_LINE_LENGTH];
                    int arg_count = 0;
                    char* token = strtok(cmd, " \t\n"); // Tokenize the user command.

                    while (token != NULL) {
                        argv[arg_count++] = token;
                        token = strtok(NULL, " \t\n");
                    }
                    argv[arg_count] = NULL; // Null-terminate the argument array.

                    if (executionTimeout > 0) {
                        // Set an alarm for executionTimeout seconds.
                        alarm(executionTimeout);
                    }

                    // Execute the program specified by the user using execve.
                    if (execve(argv[0], argv, NULL) == -1) {
                        // Handle execve failure: Print an error message using perror.
                        perror("Execve failed");
                        _exit(EXIT_FAILURE);
                    }
                } else { // Parent process
                    int status;

                    if (executionTimeout > 0) {
                        alarm(executionTimeout);
                    }

                    wait(&status); // Wait for the child process to complete.
                    alarm(0);

                    // Handle the termination of the child process.
                    if (!sigint_sent) {
                        write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
                    }
                    sigint_sent = 0;
                    break; // Exit the inner loop after command execution.

                    // Check if the process terminated due to the alarm (timeout).
                    if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM) {
                        // Handle the timeout scenario here:
                        write(STDOUT_FILENO, CATCHPHRASE, strlen(CATCHPHRASE));
                    }

                    if (executionTimeout > 0) {
                        alarm(0);
                    }
                }
            } else {
                // Read and store characters into the cmd buffer.
                cmd[i++] = c;
            }
        }
    }

    return EXIT_SUCCESS;
}
