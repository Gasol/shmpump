#include"config.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<limits.h>
#include<errno.h>

#define BUF_SIZE 1024

extern void show_usage();
extern void show_version();

int main(int argc, char** argv)
{
    const char* optstr = "hm:M:v";
    char ch = 0;
    long shm_id = 0;
    key_t shm_key = IPC_PRIVATE;

    while ((ch = getopt(argc, argv, optstr)) != -1) {
        switch (ch) {
            case 'm':
            case 'M':
            {
                long lval = 0;
                if (!(lval = strtol(optarg, NULL, 10))) {
                    goto p_error;
                }

                if (ch == 'm') {
                    shm_id = lval;
                } else {
                    shm_key = lval;
                }
                break;
            }
            case 'h':
                show_usage();
                return EXIT_SUCCESS;
            case 'v':
                show_version();
                return EXIT_SUCCESS;
            default:
                abort();
                return EXIT_FAILURE;
        }
    }

    if (shm_key && shm_id) {
        fprintf(stderr, "Both shm_key and shm_id are set\n");
        return EXIT_FAILURE;
    }

    char *file = NULL;
    if (optind < argc) {
        file = argv[optind];
    }

    if (!file) {
        fprintf(stderr, "Please specify file\n");
        return EXIT_FAILURE;
    }

    FILE *fp = NULL;
    if (!(fp = fopen(file, "r"))) {
        goto p_error;
    }

    size_t size = 64 * 1024 * 1024;
    if (shm_key) {
        int oflag = IPC_CREAT | 444 | 222;
        if (!(shm_id = shmget(shm_key, size, oflag))) {
            perror(argv[0]);
        }
    }

    void *dst = NULL;
    if ((dst = shmat(shm_id, NULL, 0) == -1)) {
        goto p_error;
    }
    printf("address: %p\n", dst);

    int len = 0;
    char buf[BUF_SIZE];
    while (!feof(fp)) {
        len = fread(buf, sizeof(char), BUF_SIZE, fp);
        memcpy(dst, buf, len);
        dst += len;
    }

    if (shmdt(dst) == -1) {
        goto p_error;
    }

    return EXIT_SUCCESS;

p_error:
    perror(PACKAGE);
    return EXIT_FAILURE;
}

void show_usage()
{
    printf("usage: shmpump [-h] [-m id] [-M key] file\n");
}

void show_version()
{
    printf("%s\nReport bug to: %s\n", PACKAGE_STRING, PACKAGE_BUGREPORT);
}
