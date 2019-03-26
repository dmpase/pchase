#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "Main.h"
#include "SHA.h"


int
main(int argc, const char* argv[])
{
    const char* msg = NULL;
    uint32_t msg_bytes = 0;
    sha256_digest_t digest;

    if (argc == 3 && strcmp("-f", argv[1]) == 0) {
        const char* file_name = argv[2];
	int fd = open(file_name, O_RDONLY);
	if (fd < 0) {
	    fprintf(stderr, "Unable to open the file for reading, exiting.\n");
	    exit(1);
	}
        msg_bytes = lseek(fd, 0, SEEK_END);
        char* tmp = new char[msg_bytes];
        lseek(fd, 0, SEEK_SET);
        read(fd, tmp, msg_bytes);
        msg = tmp;
    } else if (argc == 3 && strcmp("-s", argv[1]) == 0) {
        msg = argv[2];
        msg_bytes = strlen(msg);
    } else {
        printf("usage: %s -f <file_name>\n", argv[0]);
        printf("       %s -s <text>\n", argv[0]);
        exit(1);
    }

    sha256_compute((char*)msg, 8*msg_bytes, &digest);
    sha256_print_digest(&digest);
}

const char* Main_C = "\0@ID " ID;
