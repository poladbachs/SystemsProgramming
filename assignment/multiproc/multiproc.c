/*
 * multiproc.c
 *
 * A solution that strictly follows the assignment requirements and addresses the failing tests.
 *
 * Author: [Your Name]
 * Acknowledgments:
 * - Adapted from user-provided code.
 * - Referenced Advanced Programming in the UNIX Environment (3rd Ed., W. Stevens and S. Rago, Addison-Wesley, 2013).
 *
 * Limitations:
 * - Assumes that the processing and summarization programs handle line buffering correctly.
 * - Does not implement signal handling beyond basic error checking.
 */

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>

#define DEFAULT_PROC "cat"
#define DEFAULT_SUMM "cat"
#define DEFAULT_WORKERS 5
#define MAX_WORKERS 20
#define MAX_LINE_LENGTH 1024

typedef struct FileNode {
    char *path;
    struct FileNode *next;
} FileNode;

typedef struct Worker {
    pid_t pid;
    int fd;
    char buffer[MAX_LINE_LENGTH + 1]; // +1 for potential newline
    int buf_pos;
} Worker;

static FileNode *file_list_head = NULL;
static FileNode *file_list_tail = NULL;

static char *proc_prog = NULL;
static char *summ_prog = NULL;
static int workers = DEFAULT_WORKERS;

static pid_t summary_pid = -1;
static int summary_stdin_fd = -1;
static int nfiles = 0;

/* Function to print the help message */
static void print_help(FILE *out) {
    /* EXACT help message as specified by the user (expected by tests) */
    fprintf(out,
"Usage: ./multiproc [options] [file]...\n"
"\n"
"Arguments:\n"
"  [file]...       The list of regular files on which the program should\n"
"                  apply the processing and summarize operations. The\n"
"                  list can also contain some directories. In case of a\n"
"                  directory, the program will apply the processing and\n"
"                  summarizing operation on all regular files within the\n"
"                  directory and its sub-directories.\n"
"\n"
"Options:\n"
"  -p <prog>, --proc <prog>             The program for the processing\n"
"                                       operation (default: cat)\n"
"  -s <prog>, --summary <prog>          The program for the summarize\n"
"                                       operation (default: cat)\n"
"  -w <workers>, --workers <workers>    The number of forked processes\n"
"                                       (default: 5, max: 20)\n"
"  -h, --help                           Display this help and exit\n");
}

/* Function to free the file list */
static void free_file_list(void) {
    FileNode *curr = file_list_head;
    while (curr) {
        FileNode *tmp = curr;
        curr = curr->next;
        free(tmp->path);
        free(tmp);
    }
    file_list_head = NULL;
    file_list_tail = NULL;
}

/* Function to clean up resources and exit */
static void cleanup_and_exit(int status, Worker *worker_list, int worker_count) {
    // Free the file list
    free_file_list();

    // Terminate all worker processes
    for (int i = 0; i < worker_count; i++) {
        if (worker_list[i].pid > 0) {
            kill(worker_list[i].pid, SIGTERM);
            waitpid(worker_list[i].pid, NULL, 0);
            if (worker_list[i].fd >= 0) {
                close(worker_list[i].fd);
            }
        }
    }
    free(worker_list);

    // Terminate summarizer process
    if (summary_pid > 0) {
        kill(summary_pid, SIGTERM);
        waitpid(summary_pid, NULL, 0);
    }

    // Close summarizer's stdin if open
    if (summary_stdin_fd >= 0) {
        close(summary_stdin_fd);
    }

    exit(status);
}

/* Function to check if a path is a regular file */
static int is_regular_file(const char *path) {
    struct stat st;
    if (stat(path, &st) < 0) return -1;
    return S_ISREG(st.st_mode) ? 1 : 0;
}

/* Function to check if a path is a directory */
static int is_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) < 0) return -1;
    return S_ISDIR(st.st_mode) ? 1 : 0;
}

/* Function to add a file to the file list */
static int add_file(const char *path) {
    FileNode *node = malloc(sizeof(FileNode));
    if (!node) return -1;
    node->path = strdup(path);
    if (!node->path) {
        free(node);
        return -1;
    }
    node->next = NULL;
    if (!file_list_head) {
        file_list_head = node;
        file_list_tail = node;
    } else {
        file_list_tail->next = node;
        file_list_tail = node;
    }
    nfiles++;
    return 0;
}

/* Function to recursively add files from a directory */
static int add_directory_recursive(const char *dirpath) {
    DIR *d = opendir(dirpath);
    if (!d) {
        fprintf(stderr, "Failed to process: %s\n", dirpath);
        return -1;
    }
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        char path[PATH_MAX];
        if ((strlen(dirpath) + 1 + strlen(ent->d_name)) >= sizeof(path)) {
            closedir(d);
            fprintf(stderr, "Failed to process: %s/%s\n", dirpath, ent->d_name);
            return -1;
        }
        snprintf(path, sizeof(path), "%s/%s", dirpath, ent->d_name);
        int dcheck = is_directory(path);
        if (dcheck < 0) {
            closedir(d);
            fprintf(stderr, "Failed to process: %s\n", path);
            return -1;
        }
        if (dcheck == 1) {
            if (add_directory_recursive(path) < 0) {
                closedir(d);
                return -1;
            }
        } else {
            int fcheck = is_regular_file(path);
            if (fcheck < 0) {
                closedir(d);
                fprintf(stderr, "Failed to process: %s\n", path);
                return -1;
            }
            if (fcheck == 1) {
                if (add_file(path) < 0) {
                    closedir(d);
                    fprintf(stderr, "Failed to process: %s\n", path);
                    return -1;
                }
            } 
        }
    }
    closedir(d);
    return 0;
}

/* Function to collect files from command-line arguments */
static int collect_files(char **paths, int count) {
    for (int i = 0; i < count; i++) {
        const char *p = paths[i];
        int dcheck = is_directory(p);
        if (dcheck < 0) {
            int fcheck = is_regular_file(p);
            if (fcheck < 0) {
                fprintf(stderr, "Failed to process: %s\n", p);
                return -1; // can't stat, error
            }
            if (fcheck == 1) {
                if (add_file(p) < 0) {
                    fprintf(stderr, "Failed to process: %s\n", p);
                    return -1;
                }
            }
            // else ignore non-regular files
        } else if (dcheck == 1) {
            // directory
            if (add_directory_recursive(p) < 0) return -1;
        } else {
            // not directory, check file
            int fcheck = is_regular_file(p);
            if (fcheck < 0) {
                fprintf(stderr, "Failed to process: %s\n", p);
                return -1;
            }
            if (fcheck == 1) {
                if (add_file(p) < 0) {
                    fprintf(stderr, "Failed to process: %s\n", p);
                    return -1;
                }
            }
            // else ignore non-regular files
        }
    }
    return 0;
}

/* Function to parse command-line arguments */
static int parse_args(int argc, char **argv) {
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(stdout);
            exit(EXIT_SUCCESS);
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--proc") == 0) {
            i++;
            if (i >= argc) {
                print_help(stderr);
                return -1;
            }
            proc_prog = argv[i];
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--summary") == 0) {
            i++;
            if (i >= argc) {
                print_help(stderr);
                return -1;
            }
            summ_prog = argv[i];
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--workers") == 0) {
            i++;
            if (i >= argc) {
                print_help(stderr);
                return -1;
            }
            char *endptr = NULL;
            long w = strtol(argv[i], &endptr, 10);
            if (*endptr != '\0' || w < 1 || w > MAX_WORKERS) {
                fprintf(stderr, "Invalid number of workers provided. The number of workers must be > 0 and <= 20\n");
                return -1;
            }
            workers = (int)w;
        } else if (argv[i][0] == '-') {
            // invalid option
            print_help(stderr);
            return -1;
        } else {
            break;
        }
        i++;
    }

    if (!proc_prog) proc_prog = DEFAULT_PROC;
    if (!summ_prog) summ_prog = DEFAULT_SUMM;

    int count_files = argc - i;
    if (count_files <= 0) {
        // zero files => no output => success
        exit(EXIT_SUCCESS);
    }

    if (collect_files(&argv[i], count_files) < 0) {
        // Error message already printed in collect_files
        return -1;
    }

    return 0;
}

/* Function to start the summarizer process */
static void start_summarizer_process(void) {
    int fd_pipe[2];
    if (pipe(fd_pipe) < 0) {
        fprintf(stderr, "Failed to create pipe for summarizer\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Failed to fork summarizer process\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process: summarizer
        close(fd_pipe[1]); // Close write end
        if (dup2(fd_pipe[0], STDIN_FILENO) < 0) {
            _exit(EXIT_FAILURE);
        }
        close(fd_pipe[0]);
        execlp(summ_prog, summ_prog, (char *)NULL);
        _exit(EXIT_FAILURE);
    }

    // Parent process
    close(fd_pipe[0]); // Close read end
    summary_pid = pid;
    summary_stdin_fd = fd_pipe[1];
}

/* Function to start a worker process */
static int start_worker(const char *filename, Worker *worker) {
    int fd_pipe[2];
    if (pipe(fd_pipe) < 0) {
        fprintf(stderr, "Failed to create pipe for worker\n");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Failed to fork worker process\n");
        close(fd_pipe[0]);
        close(fd_pipe[1]);
        return -1;
    }

    if (pid == 0) {
        // Child process: worker
        close(fd_pipe[0]); // Close read end
        if (dup2(fd_pipe[1], STDOUT_FILENO) < 0) {
            _exit(EXIT_FAILURE);
        }
        close(fd_pipe[1]);
        execlp(proc_prog, proc_prog, filename, (char *)NULL);
        _exit(EXIT_FAILURE);
    }

    // Parent process
    close(fd_pipe[1]); // Close write end
    worker->pid = pid;
    worker->fd = fd_pipe[0];
    worker->buf_pos = 0;
    return 0;
}

/* Function to safely write all data to a file descriptor */
static int safe_write_all(int fd, const void *buf, size_t count) {
    const char *p = buf;
    size_t left = count;
    while (left > 0) {
        ssize_t w = write(fd, p, left);
        if (w < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        p += w;
        left -= w;
    }
    return 0;
}

/* Function to safely read data from a file descriptor */
static ssize_t safe_read_all(int fd, void *buf, size_t count) {
    ssize_t r;
    do {
        r = read(fd, buf, count);
    } while (r < 0 && errno == EINTR);
    return r;
}

int main(int argc, char **argv) {
    if (parse_args(argc, argv) < 0) {
        // Specific error message already printed.
        exit(EXIT_FAILURE);
    }

    if (nfiles == 0) {
        // no files
        exit(EXIT_SUCCESS);
    }

    start_summarizer_process();

    // Initialize worker list
    Worker *workers_list = calloc(workers, sizeof(Worker));
    if (!workers_list) {
        fprintf(stderr, "Failed to allocate memory for workers\n");
        cleanup_and_exit(EXIT_FAILURE, NULL, 0);
    }

    int active_workers = 0;
    FileNode *current_file = file_list_head;

    // Start initial workers
    for (int i = 0; i < workers && current_file; i++) {
        if (start_worker(current_file->path, &workers_list[i]) < 0) {
            // Specific error message already printed in start_worker
            cleanup_and_exit(EXIT_FAILURE, workers_list, workers);
        }
        active_workers++;
        current_file = current_file->next;
    }

    while (active_workers > 0) {
        fd_set rfds;
        FD_ZERO(&rfds);
        int maxfd = -1;

        for (int i = 0; i < workers; i++) {
            if (workers_list[i].pid > 0 && workers_list[i].fd >= 0) {
                FD_SET(workers_list[i].fd, &rfds);
                if (workers_list[i].fd > maxfd) {
                    maxfd = workers_list[i].fd;
                }
            }
        }

        if (maxfd == -1) {
            break; // No active fds
        }

        int sel = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (sel < 0) {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "Failed during select\n");
            cleanup_and_exit(EXIT_FAILURE, workers_list, workers);
        }

        for (int i = 0; i < workers; i++) {
            if (workers_list[i].pid > 0 && workers_list[i].fd >= 0 && FD_ISSET(workers_list[i].fd, &rfds)) {
                char c;
                ssize_t r = safe_read_all(workers_list[i].fd, &c, 1);
                if (r < 0) {
                    fprintf(stderr, "Failed to read from worker\n");
                    cleanup_and_exit(EXIT_FAILURE, workers_list, workers);
                } else if (r == 0) {
                    // EOF
                    close(workers_list[i].fd);
                    workers_list[i].fd = -1;

                    // Wait for worker to terminate
                    int status;
                    if (waitpid(workers_list[i].pid, &status, 0) < 0) {
                        fprintf(stderr, "Failed to wait for worker process\n");
                        cleanup_and_exit(EXIT_FAILURE, workers_list, workers);
                    }

                    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                        fprintf(stderr, "Failed to process: %s\n", file_list_head->path);
                        cleanup_and_exit(EXIT_FAILURE, workers_list, workers);
                    }

                    active_workers--;

                    // Start a new worker if files are remaining
                    if (current_file) {
                        if (start_worker(current_file->path, &workers_list[i]) < 0) {
                            // Specific error message already printed in start_worker
                            cleanup_and_exit(EXIT_FAILURE, workers_list, workers);
                        }
                        active_workers++;
                        current_file = current_file->next;
                    }

                    // If there was a partial line, append newline and send to summarizer
                    if (workers_list[i].buf_pos > 0) {
                        workers_list[i].buffer[workers_list[i].buf_pos] = '\n';
                        workers_list[i].buf_pos++;
                        if (safe_write_all(summary_stdin_fd, workers_list[i].buffer, workers_list[i].buf_pos) < 0) {
                            fprintf(stderr, "Failed to write to summarizer\n");
                            cleanup_and_exit(EXIT_FAILURE, workers_list, workers);
                        }
                        workers_list[i].buf_pos = 0;
                    }
                } else {
                    // Read one character
                    if (workers_list[i].buf_pos < MAX_LINE_LENGTH) {
                        workers_list[i].buffer[workers_list[i].buf_pos] = c;
                        workers_list[i].buf_pos++;
                        if (c == '\n') {
                            if (safe_write_all(summary_stdin_fd, workers_list[i].buffer, workers_list[i].buf_pos) < 0) {
                                fprintf(stderr, "Failed to write to summarizer\n");
                                cleanup_and_exit(EXIT_FAILURE, workers_list, workers);
                            }
                            workers_list[i].buf_pos = 0;
                        }
                    }

                    if (workers_list[i].buf_pos >= MAX_LINE_LENGTH) {
                        // Flush partial line
                        if (safe_write_all(summary_stdin_fd, workers_list[i].buffer, workers_list[i].buf_pos) < 0) {
                            fprintf(stderr, "Failed to write to summarizer\n");
                            cleanup_and_exit(EXIT_FAILURE, workers_list, workers);
                        }
                        workers_list[i].buf_pos = 0;
                    }
                }
            }
        }
    }

    // After all workers are done, close summarizer stdin
    close(summary_stdin_fd);
    summary_stdin_fd = -1;

    // Wait for summarizer process to terminate
    int summ_status;
    if (waitpid(summary_pid, &summ_status, 0) < 0) {
        fprintf(stderr, "Failed to wait for summarizer process\n");
        cleanup_and_exit(EXIT_FAILURE, workers_list, workers);
    }
    if (!WIFEXITED(summ_status) || WEXITSTATUS(summ_status) != 0) {
        fprintf(stderr, "Summarizer process failed\n");
        cleanup_and_exit(EXIT_FAILURE, workers_list, workers);
    }

    // Free worker list
    free(workers_list);

    return EXIT_SUCCESS;
}