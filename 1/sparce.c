#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#define DEFAULT_BLOCK_SIZE 4096

void error_exit(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int is_zero_block(const unsigned char *buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (buffer[i] != 0) {
            return 0;
        }
    }
    return 1;
}

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [-b block_size] output_file\n", program_name);
    fprintf(stderr, "              %s [-b bolck_size] input_file output_file\n", program_name);
    fprintf(stderr, "Arguments:\n");
    fprintf(stderr, "  -b SIZE   Define block size in bytes (default: %d)\n", DEFAULT_BLOCK_SIZE);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int block_size = DEFAULT_BLOCK_SIZE;
    int opt;
    
    while ((opt = getopt(argc, argv, "b:")) != -1) {
        switch (opt) {
            case 'b':
                block_size = atoi(optarg);
                if (block_size <= 0) {
                    fprintf(stderr, "Block size must be positive\n");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                print_usage(argv[0]);
        }
    }
    
    int input_fd, output_fd;
    int args_left = argc - optind;
    
    if (args_left == 1) {
        input_fd = STDIN_FILENO;
        output_fd = open(argv[optind], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (output_fd == -1) {
            error_exit("Can't open output file");
        }
    } else if (args_left == 2) {
        input_fd = open(argv[optind], O_RDONLY);
        if (input_fd == -1) {
            error_exit("Can't open input file");
        }
        
        output_fd = open(argv[optind + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (output_fd == -1) {
            close(input_fd);
            error_exit("Can't open output file");
        }
    } else {
        print_usage(argv[0]);
    }
    
    unsigned char *buffer = malloc(block_size);
    if (!buffer) {
        error_exit("Memory allocation error");
    }
    
    ssize_t bytes_read;
    off_t total_size = 0;
    
    while ((bytes_read = read(input_fd, buffer, block_size)) > 0) {
        if (is_zero_block(buffer, bytes_read)) {
            if (lseek(output_fd, bytes_read, SEEK_CUR) == -1) {
                free(buffer);
                error_exit("lseek error");
            }
        } else {
            ssize_t bytes_written = write(output_fd, buffer, bytes_read);
            if (bytes_written != bytes_read) {
                free(buffer);
                error_exit("Can't write in file");
            }
        }
        total_size += bytes_read;
    }
    
    if (bytes_read == -1) {
        free(buffer);
        error_exit("Can't read from file");
    }
    
    if (ftruncate(output_fd, total_size) == -1) {
        free(buffer);
        error_exit("ftruncate error");
    }
    
    free(buffer);
    
    if (input_fd != STDIN_FILENO) {
        close(input_fd);
    }
    close(output_fd);
    
    return 0;
}
