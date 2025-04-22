#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile int locks = 0;

static void stop() {
    int fd_stat = open("result.txt", O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    int pid = getpid();
    char stat[50] = "\0";
    sprintf(stat, "%d - PID[%d]\n", locks, pid);
    write(fd_stat, stat, strlen(stat));
    close(fd_stat);
    exit(0);
}

static void print_usage(const char *program_name) {
    fprintf(stderr,
            "Usage: %s -f <file_to_lock>\n"
            "  -f <file>   file to lock",
            program_name);
    exit(EXIT_FAILURE);
}

void main(int argc, char *argv[]) {

    char *filename = NULL;
    int opt;

    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
        case 'f':
            filename = optarg;
            break;
        default:
            print_usage(argv[0]);
        }
    }
    if (!filename) {
        print_usage(argv[0]);
    }

    signal(SIGINT, stop);

    char *filename_lck = malloc((sizeof(char) * strlen(filename)) + 5);
    strcpy(filename_lck, filename);
    strcat(filename_lck, ".lck");

    while (1) {
        pid_t pid = getpid();
        int fd_lck = open(filename_lck, O_CREAT | O_EXCL | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd_lck == -1) {
            sleep(1);
            continue;
        }
        write(fd_lck, &pid, sizeof(int));
        close(fd_lck);

        FILE *fd = fopen(filename, "r+");
        sleep(1);
        fclose(fd);


        fd_lck = open(filename_lck, O_RDONLY);
        if (fd_lck == -1) {
            exit(errno);
        } 
        int actual_pid = 0;
        read(fd_lck, &actual_pid, sizeof(int));
        if (actual_pid != pid) {
            close(fd_lck);
            exit(errno);
        }
        
        locks++;
        close(fd_lck);         
        remove(filename_lck);
    }
}
