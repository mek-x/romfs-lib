#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <argp.h>
#include <stdbool.h>
#include <romfs.h>


#define FATAL(msg, ...) { printf("Fatal: " msg "\n", ##__VA_ARGS__); return 1; }


const char *argp_program_version = "v" ROMFS_VERSION;
static char doc[] = "Small tool to parse files in romfs image.";
static char args_doc[] = "FILENAME";

static struct argp_option options[] = {
    { "list", 'l', 0, OPTION_ARG_OPTIONAL, "List files in given path. If no path given, list files in root. Default mode."},
    { "read", 'r', 0, OPTION_ARG_OPTIONAL, "Read contents of the file specified in path."},
    { "path", 'p', "PATH", OPTION_ARG_OPTIONAL, "Path in romfs."},
    { 0 }
};

struct arguments {
    enum { LIST_MODE, READ_MODE } mode;
    char *path;
    char *file;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
        case 'l': arguments->mode = LIST_MODE; break;
        case 'r': arguments->mode = READ_MODE; break;
        case 'p': arguments->path = arg; break;
        case ARGP_KEY_ARG: return 0;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

static
int OpenRomfs(const char * filename, uint8_t **romfs, size_t *romfs_size) {
    long filesize;
    FILE *f = fopen(filename, "rb");
    if (NULL == f) return 1;

    fseek(f, 0L, SEEK_END);
    filesize = ftell(f);
    rewind(f);

    *romfs = (uint8_t *)mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fileno(f), 0);
    if (*romfs == MAP_FAILED) return 1;

    fclose(f);

    *romfs_size = filesize;

    return 0;
}

#define DIR_BUF_LEN 100

static
int ListDir(romfs_t r, const char *path) {
    int f, ret;
    romfs_dirent_t dir[DIR_BUF_LEN];
    uint32_t cookie = 0;
    size_t used = DIR_BUF_LEN;
    romfs_stat_t stat;
    size_t total = 0;

    f = RomfsOpenRoot(r, path, 0);
    if (f < 0) return f;

    do {
        ret = RomfsReadDir(r, f, dir, DIR_BUF_LEN, &cookie, &used);
        if (ret < 0) return ret;

        printf("[%-10s] [%-20s] [%-4s] [%-4s] [%-10s]\n", "Offset", "Name", "Mode", "Size", "Check");
        for (size_t i = 0; i < used; i++) {
            printf("[0x%08x] %20s", dir[i].inode, dir[i].name);
            if (IS_DIRECTORY(dir[i].type)) putchar('/');
            else if (IS_FILE(dir[i].type) && IS_EXEC(dir[i].type)) putchar('*');
            else putchar(' ');
            printf("   0x%02x", dir[i].type);

            if (IS_FILE(dir[i].type)) {
                ret = RomfsFdStatAt(r, f, dir[i].name, &stat);
                if (ret >= 0) {
                    printf("   %4d   0x%x", stat.size, stat.chksum);
                    total += stat.size;
                }
            }
            putchar('\n');
        }

        if (cookie == ROMFS_COOKIE_LAST) break;
    } while (used == DIR_BUF_LEN);

    printf("\nTotal size: %zu\n", total);

    RomfsClose(r, f);

    return 0;
}

#define BUF_SIZE 255

int ReadFile(romfs_t r, const char *path) {
    int f, ret;
    char buf[BUF_SIZE];

    f = RomfsOpenRoot(r, path, 0);
    if (f < 0) return f;

    do {
        ret = RomfsRead(r, f, buf, BUF_SIZE);
        if (ret < 0) return ret;

        fwrite(buf, 1, ret, stdout);
        if (ret < BUF_SIZE) break;
    } while (ret == BUF_SIZE);

    RomfsClose(r, f);
    return 0;
}

int main(int argc, char *argv[])
{
    struct arguments arguments;
    uint8_t *romfs_img;
    size_t romfs_size;
    romfs_t romfs;
    int ret;

    arguments.path = "/";
    arguments.mode = LIST_MODE;

    argp_parse(&argp, argc, argv, ARGP_NO_ARGS, &ret, &arguments);

    if (ret >= argc) FATAL("filename must be given!");
    arguments.file = argv[ret];

    if (OpenRomfs(arguments.file, &romfs_img, &romfs_size) != 0) FATAL("can't open file: %s", arguments.file);

    ret = RomfsLoad(romfs_img, romfs_size, romfs);
    if (ret < 0) { errno = -ret; perror("RomfsLoad"); return 1; }

    switch (arguments.mode)
    {
    case LIST_MODE:
        printf("%s:\n", arguments.path);
        ret = ListDir(romfs, arguments.path);
        if (ret < 0) { errno = -ret; perror("ListDir"); return 1; }
        break;
    case READ_MODE:
        ret = ReadFile(romfs, arguments.path);
        if (ret < 0) { errno = -ret; perror("ReadFile"); return 1; }
        break;
    default:
        break;
    }

    return 0;
}
