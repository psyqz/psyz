#include <psyz.h>
#include <log.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <kernel.h>

static char* path_join(char* left, const char* right, int maxlen) {
    size_t left_len = strlen(left);
    if (left_len >= maxlen - 1) {
        return NULL;
    }
    if (left[left_len - 1] != '/' && right[0] != '/') {
        if (left_len < maxlen - 1) {
            left[left_len] = '/';
            left[left_len + 1] = '\0';
            left_len++;
        } else {
            return NULL;
        }
    } else if (left[left_len - 1] == '/' && right[0] == '/') {
        // Avoid double '/'
        left[left_len - 1] = '\0';
        left_len--;
    }
    strncpy(left + left_len, right, maxlen - left_len - 1);
    left[maxlen - 1] = '\0'; // Ensure null-termination
    return left;
}

static void adjust_path(char* dst, const char* src, int maxlen) {
    size_t len = strlen(src);
    if (len >= 5 && src[0] == 'b' && src[1] == 'u' && src[4] == ':') {
        // adjust memory card path
        strncpy(dst, src, maxlen);
        dst[4] = '\0';
        struct stat st = {0};
        if (stat(dst, &st) == -1) {
            mkdir(dst, 0755);
        }
        dst[4] = '/';
        if (dst[5] == '\0' || dst[5] == '*') { // handles 'bu00:*'
            dst[5] = '\0';
        }
        return;
    } else {
        strncpy(dst, src, maxlen);
        dst[maxlen - 1] = '\0';
    }
}

static void populate_entry(
    const char* baseDir, struct DIRENTRY* dst, struct dirent* src) {
    char buf[512];
    struct stat fileStat = {0};
    strncpy(buf, baseDir, sizeof(buf));
    if (!path_join(buf, src->d_name, sizeof(buf))) {
        ERRORF("failed to join '%s' and '%s': strings are too large", baseDir,
               src->d_name);
        return;
    }
    if (stat(buf, &fileStat) != 0) {
        ERRORF("failed to stat '%s'", buf);
        return;
    }

    // TODO: names longer than 20 characters are not supported
    if (strlen(src->d_name) >= sizeof(dst->name) - 1) {
        WARNF("dir name '%s' will be truncated", src->d_name);
    }
    strncpy(dst->name, src->d_name, sizeof(dst->name) - 1);
    dst->name[sizeof(dst->name) - 1] = '\0';
    dst->attr = 0x10 | 0x40; // not sure what this is
    dst->size = fileStat.st_size;
    dst->next = NULL;
    dst->head = 0;
    dst->system[0] = 0;
}

typedef struct {
    char base_dir[1024];
    DIR* dir;
    struct dirent* last_entry;
} DIRENTRY_RESERVED;
static DIRENTRY_RESERVED singleton_dir = {0};
static bool is_filesearch_handle_open() { return singleton_dir.dir != NULL; }
static void close_filesearch_handle() {
    if (is_filesearch_handle_open()) {
        closedir(singleton_dir.dir);
        singleton_dir.base_dir[0] = '\0';
        singleton_dir.dir = NULL;
        singleton_dir.last_entry = NULL;
    }
}
static DIRENTRY_RESERVED* open_filesearch_handle(const char* basePath) {
    if (is_filesearch_handle_open()) {
        WARNF("previous firstfile at '%s' was not closed",
              singleton_dir.base_dir);
        close_filesearch_handle();
    }
    strncpy(singleton_dir.base_dir, basePath, sizeof(singleton_dir.base_dir));
    singleton_dir.dir = opendir(singleton_dir.base_dir);
    if (singleton_dir.dir) {
        struct dirent* entry;
        do {
            entry = readdir(singleton_dir.dir);
            if (!entry) {
                close_filesearch_handle();
                return NULL;
            }
        } while (entry->d_name[0] == '.' || entry->d_type != DT_REG);
        singleton_dir.last_entry = entry;
    }
    return (DIRENTRY_RESERVED*)&singleton_dir;
}
static struct dirent* read_filesearch_handle() {
    if (!singleton_dir.dir) {
        return NULL;
    }
    struct dirent* cur = singleton_dir.last_entry;
    if (!cur) {
        close_filesearch_handle();
        return NULL;
    }
    singleton_dir.last_entry = readdir(singleton_dir.dir);
    return cur;
}
struct DIRENTRY* my_firstfile(char* dirPath, struct DIRENTRY* firstEntry) {
    char basePath[0x100];
    adjust_path(basePath, dirPath, sizeof(basePath));
    DEBUGF("opendir('%s')", basePath);
    DIRENTRY_RESERVED* handle = open_filesearch_handle(basePath);
    if (!handle) {
        return NULL;
    }
    struct dirent* entry = read_filesearch_handle();
    if (!entry) {
        return NULL;
    }
    populate_entry(handle->base_dir, firstEntry, entry);
    return firstEntry;
}

struct DIRENTRY* my_nextfile(struct DIRENTRY* outEntry) {
    if (!outEntry) {
        return NULL;
    }
    struct dirent* entry = read_filesearch_handle();
    while (entry) {
        if (entry->d_type != DT_REG) {
            entry = read_filesearch_handle();
            continue;
        }
        DIRENTRY_RESERVED* handle = &singleton_dir;
        populate_entry(handle->base_dir, outEntry, entry);
        return outEntry;
    }
    return NULL;
}

long my_format(char* fs) {
    size_t path_end;
    char path[0x200];
    adjust_path(path, fs, sizeof(path));
    path_end = strlen(path);
    DEBUGF("format('%s')", fs);
    DIR* dir = opendir(path);
    struct dirent* entry;
    while ((entry = readdir(dir)) != 0) {
        if (entry->d_type != DT_REG) {
            continue;
        }
        strncpy(path + path_end, entry->d_name, sizeof(path) - path_end);
        if (remove(path)) {
            return 0;
        }
    }
    return 1;
}

long my_erase(char* path) {
    char adjPath[0x100];
    adjust_path(adjPath, path, sizeof(adjPath));

    DEBUGF("remove('%s')", adjPath);
    return remove(adjPath) == 0;
}

#undef open
#undef close
#undef lseek
#undef read
#undef write
#undef ioctl
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
int my_open(const char* devname, int flag) {
    // only map the known flags and discard the rest
    int oflag = (int)flag & (O_WRONLY | O_RDWR | O_NONBLOCK | O_CREAT);
    char path[0x100];
    adjust_path(path, devname, sizeof(path));
    if (oflag & O_CREAT) {
        return creat(path, 0644);
    } else {
        struct stat st;
        if (stat(path, &st) != 0) {
            WARNF("path '%s' mapped from '%s' not found", path, devname);
            return -1;
        }
        if (!(st.st_mode & S_IFREG)) {
            if (st.st_mode & S_IFDIR) {
                WARNF(
                    "path '%s' mapped from '%s' is a directory", path, devname);
            } else {
                WARNF("path '%s' mapped from '%s' is not a regular file", path,
                      devname);
            }
            return -1;
        }
        return open(path, oflag);
    }
}
int my_close(int fd) { return (long)close((int)fd); }
long my_lseek(long fd, long offset, long flag) {
    return lseek((int)fd, (off_t)offset, (int)flag);
}
long my_read(long fd, void* buf, long n) {
    return (long)read((int)fd, buf, (size_t)n);
}
long my_write(long fd, void* buf, long n) {
    return (long)write((int)fd, buf, (size_t)n);
}
long my_ioctl(long fd, long com, long arg) { return ioctl((int)fd, com, arg); }
