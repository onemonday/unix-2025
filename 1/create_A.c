#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    const int size = 4*1024*1024+1;

    char *buffer = calloc(size, 1);
    buffer[0]=1;
    buffer[9999]=1;
    buffer[size-1]=1;
    
    int fd = open("A", O_TRUNC | O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, "Can't open file");
        return 1;
    }
    if (write(fd, buffer, size) == -1) {
        fprintf(stderr, "Can't write in file");
        return 1;
    }

    return 0;
}