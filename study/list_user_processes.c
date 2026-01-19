#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <pwd.h>

__uid_t user_id_from_name(const char *name)
{
    struct passwd *pwd;
    __uid_t u;
    char *endptr;

    if (!name || *name == '\0')
        return -1;

    u = strtol(name, &endptr, 10);
    if (*endptr == '\0')
        return u;

    pwd = getpwnam(name);
    if (!pwd)
        return -1;

    return pwd->pw_uid;
}

int main(int argc, char *argv[])
{
    __uid_t uid;
    DIR *dir;
    struct dirent *entry = NULL;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <username|uid>\n", argv[0]);
        return EXIT_FAILURE;
    }

    uid = user_id_from_name(argv[1]);
    if (uid == (__uid_t)-1) {
        fprintf(stderr, "User not found: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    if ((dir = opendir("/proc")) == NULL) {
        perror("opendir /proc");
        return EXIT_FAILURE;
    }

    while ((entry = readdir(dir)) != NULL) {
        char buf[1024] = {0, };
        char name[256] = {0, };
        char status_path[1024] = {0, };
        __uid_t proc_uid;
        FILE *fp;
        
        if (entry->d_name[0] < '0' || entry->d_name[0] > '9')
            continue;

        snprintf(status_path, sizeof(status_path), "/proc/%s/status", entry->d_name);
        if ((fp = fopen(status_path, "r")) != NULL) {
            while (fgets(buf, sizeof(buf), fp)) {
                sscanf(buf, "Name: %s", name);
                if (sscanf(buf, "Uid: %u", &proc_uid) == 1) {
                    if (proc_uid == uid) {
                        fprintf(stdout, "PID: %s, Name: %s\n", entry->d_name, name);
                        break;
                    }
                }
            }
        }
    }

    closedir(dir);
    return EXIT_SUCCESS;
}