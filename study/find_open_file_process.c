#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return -1;
    }

    const char *file_path = argv[1];
    DIR *dir = NULL;
    struct dirent *entry;
    int ret = 0;

    dir = opendir("/proc");
    if (!dir) {
        perror("opendir /proc");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] < '1' || entry->d_name[0] > '9')
            continue;

        DIR *fd_dir;
        struct dirent *fd_entry;
        char fd_path[256];
        snprintf(fd_path, sizeof(fd_path), "/proc/%s/fd", entry->d_name);

        fd_dir = opendir(fd_path);
        if (!fd_dir) {
            perror("opendir fd_dir");
            ret = -1;
            break;
        }

        while ((fd_entry = readdir(fd_dir)) != NULL) {
            if (fd_entry->d_name[0] == '.')
                continue;

            char link_path[256] = {0, };
            char target_path[256] = {0, };
            snprintf(link_path, sizeof(link_path), "%s/%s", fd_path, fd_entry->d_name);
            if (readlink(link_path, target_path, sizeof(target_path) - 1) == -1) {
                perror("readlink");
                break;
            }

            if (!strcmp(target_path, file_path)) {
                char comm_path[256] = {0, };
                snprintf(comm_path, sizeof(comm_path), "/proc/%s/comm", entry->d_name);
                int comm_fd = open(comm_path, O_RDONLY);
                if (comm_fd == -1) {
                    perror("open comm");
                    break;
                }

                char comm_buf[256] = {0, };
                if (read(comm_fd, comm_buf, sizeof(comm_buf) - 1) == -1) {
                    perror("read comm");
                    close(comm_fd);
                    break;
                }
                close(comm_fd);

                fprintf(stdout, "PID: %s, CMD: %s", entry->d_name, comm_buf);
            }
        }

        closedir(fd_dir);
    }


    closedir(dir);
    return ret;
}