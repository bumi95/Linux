#define _GNU_SOURCE
//#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>

struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

struct process_tree {
    int pid;
    char cmd[256];
    struct list_head children;
    struct list_head sibling;
    struct list_head list;
};

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

//LIST_HEAD(process_tree_list, process_tree);

int is_empty(struct list_head *head) {
    return head->next == head;
}

void list_init(struct list_head *head) {
    head->next = head;
    head->prev = head;
}

void list_add_tail(struct list_head *new, struct list_head *head) {
    new->prev = head->prev;
    new->next = head;
    head->prev->next = new;
    head->prev = new;
}

void list_del(struct list_head *entry) {
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
}

struct process_tree *bfs_search(struct list_head *entry, int target_pid) {
    struct list_head *queue = (struct list_head *)malloc(sizeof(struct list_head));
    if (!queue) {
        perror("malloc");
        return NULL;
    }

    queue->next = queue;
    queue->prev = queue;

    struct process_tree *root = container_of(entry->next, struct process_tree, sibling);
    list_add_tail(&root->list, queue);

    while (!is_empty(queue)) {
        struct list_head *current = queue->next;
        list_del(current);
        
        struct process_tree *proc = container_of(current, struct process_tree, list);
        if (proc->pid == target_pid) {
            free(queue);
            return proc;
        }
        
        struct list_head *child = proc->children.next;
        while (child != &proc->children) {
            struct process_tree *child_proc = container_of(child, struct process_tree, sibling);
            list_add_tail(&child_proc->list, queue);
            //if (&proc->children == child->next)
            //    break;
            child = child->next;
        }
    }

    //fprintf(stderr, "Process with PID %d not found\n", target_pid);
    free(queue);
    return NULL;
}

void print_tree(struct process_tree *node, int level) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
    printf("|-- %s (PID: %d)\n", node->cmd, node->pid);

    struct list_head *child = node->children.next;
    while (child != &node->children) {
        struct process_tree *child_proc = container_of(child, struct process_tree, sibling);
        print_tree(child_proc, level + 1);
        child = child->next;
    }
}

int main() {
    struct list_head head;
    struct process_tree *proc = NULL;
    DIR *proc_dir;
    struct dirent *entry;

    list_init(&head);

    proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("opendir /proc");
        return EXIT_FAILURE;
    }

    while((entry = readdir(proc_dir)) != NULL) {
        char buf[1024] = {0, };
        char exe_path[1024] = {0, };
        int ppid = 0;

        if (entry->d_name[0] < '1' || entry->d_name[0] > '9')
            continue;

        struct process_tree *new = (struct process_tree *)malloc(sizeof(struct process_tree));
        if (!new) {
            perror("malloc");
            closedir(proc_dir);
            return EXIT_FAILURE;
        }

        memset(new, 0, sizeof(struct process_tree));

        list_init(&new->children);
        list_init(&new->sibling);
        list_init(&new->list);

        //snprintf(exe_path, sizeof(exe_path), "/proc/%s/exe", entry->d_name);
        new->pid = strtol(entry->d_name, NULL, 10);
        //if (readlink(exe_path, new->cmd, sizeof(new->cmd) - 1) == -1) {
        //    free(new);
        //    continue;
        //}

        memset(exe_path, 0, sizeof(exe_path));
        snprintf(exe_path, sizeof(exe_path), "/proc/%s/status", entry->d_name);
        FILE *status_file = fopen(exe_path, "r");
        if (status_file) {
            while (fgets(buf, sizeof(buf), status_file)) {
                sscanf(buf, "Name: %s", new->cmd);
                if (sscanf(buf, "PPid: %d", &ppid) == 1) {
                    break;
                }
            }

            fclose(status_file);
        }

        if (new->pid == 1) {
            list_add_tail(&new->sibling, &head);
        } else {
            struct process_tree *parent = bfs_search(&head, ppid);
            if (parent) {
                list_add_tail(&new->sibling, &parent->children);
            }
        }
    }

    struct process_tree *root = container_of(head.next, struct process_tree, sibling);
    print_tree(root, 0);

    closedir(proc_dir);
    return EXIT_SUCCESS;
}