#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

void run_command(char *cmd, char *args[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(cmd, args);
        perror("execvp failed");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Command %s failed with exit code %d\n", cmd, WEXITSTATUS(status));
            exit(1);
        }
    }
}

int main() {
    char input_dir[] = "./input";
    char output_dir[] = "./output";
    char copy_files[] = "./copy_files";
    char compare_files[] = "./compare_files";
    char initial_data[] = "./initial_data";
    char changed_data[] = "./changed_data";
    char data[] = "./Data";

    run_command(copy_files, (char *[]) {copy_files, data, changed_data, NULL});
    // exit(0);

    for (int i = 1; i <= 20; i++) {
        char input_file[256], output_file[256];
        snprintf(input_file, sizeof(input_file), "%s/input%d.txt", input_dir, i);
        snprintf(output_file, sizeof(output_file), "%s/output%d.txt", output_dir, i);

        // Step 1: Run copy_files
        run_command(copy_files, (char *[]) {copy_files, changed_data, initial_data, NULL});

        // Step 2: Prepare file descriptors
        int fd_in = open(input_file, O_RDONLY);
        if (fd_in < 0) {
            perror("Failed to open input file");
            exit(1);
        }

        int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd_out < 0) {
            perror("Failed to open output file");
            exit(1);
        }

        // Step 3: Fork and run tema
        pid_t pid = fork();
        if (pid == 0) {
            dup2(fd_in, 0);  // Redirect stdin
            dup2(fd_out, 1); // Redirect stdout
            close(fd_in);
            close(fd_out);
            char *name = "./tema";
            char *args[] = {name, changed_data, NULL};
            execvp(name, args);
            perror("Failed to exec tema");
            exit(1);
        } else {
            close(fd_in);
            close(fd_out);
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                fprintf(stderr, "tema failed with exit code %d\n", WEXITSTATUS(status));
                exit(1);
            }
        }

        // Step 4: Run compare_files
        run_command(compare_files, (char *[]) {compare_files, initial_data, changed_data, NULL});
    }

    return 0;
}
