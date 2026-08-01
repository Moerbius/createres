#include <unistd.h>

#include <stdio.h>
#include <cstdlib>
#include <ctype.h>

#include "Resource.h"

#define VERSION "3.1.0"
#define ARGUMENTS "hr:f:u:cl"

void showList();
void unpack(char *filename);
void showHelp();

char *resourcename = NULL;
char *folder = NULL;

int main(int argc, char** argv) {
    const char *arguments = ARGUMENTS;

    // Our Resource Class
    Resource resources;

    int c;
    char *unpack_filename = NULL;
    int do_list = 0;

    if (argc == 1)
        showHelp();

    c = getopt(argc, argv, arguments);

    while (c != -1)
    {
        switch (c)
        {
            case 'h':
                showHelp();
                break;
            case 'r':
                resourcename = optarg;
                break;
            case 'f':
                folder = optarg;
                break;
            case 'l':
                // Defer until all options are parsed so -r can appear after -l
                do_list = 1;
                break;
            case 'c':
                resources.compress = 1;
                break;
            case 'u':
                // Defer until all options are parsed so -r can appear after -u
                unpack_filename = optarg;
                break;
            case '?':
                if (optopt == ':')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                exit(1);
            default:
                showHelp();
                break;
        }

        c = getopt(argc, argv, arguments);
    }

    if (do_list) {
        showList();
    }

    if (unpack_filename != NULL) {
        unpack(unpack_filename);
    }

    if (resourcename != NULL && folder != NULL)
    {
        resources.pack(resourcename, folder);
    }

    for (int index = optind; index < argc; index++)
        printf("Non-option argument %s\n", argv[index]);

    return 0;
}

void unpack(char *filename) {
    int filesize;
    Resource resources;
    char *data;

    printf("Unpacking file: %s\n", filename);

    if (resourcename == NULL) {
        printf("Must specify a resource filename.\n");
        exit(1);
    }

    data = resources.unpack(resourcename, filename, &filesize);

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("Error creating output file");
        free(data);
        exit(1);
    }

    int totalwritten = 0;
    while (totalwritten < filesize) {
        ssize_t written = write(fd, data + totalwritten, filesize - totalwritten);
        if (written < 0) {
            perror("Error writing unpacked file");
            close(fd);
            free(data);
            exit(1);
        }

        totalwritten += static_cast<int>(written);
    }

    close(fd);
    free(data);

    printf("Done!\n");
}

void showList() {
    if (resourcename != NULL) {
        Resource resources;
        int status = resources.listFiles(resourcename);
        if (status != 0) {
            exit(1);
        }
    }
    else {
        printf("Must specify a resource filename.");
        exit(1);
    }
}

void showHelp() {
    printf("\n");
    printf("Resource Packer %s\n", VERSION);
    printf("Copyright 2014 - 2026 by Moerbius\n\n");
    printf("Usage:\n");
    printf("createres <options>\n\n");
    printf("   -h                  This help\n");
    printf("   -r <resource name>  Resource file name\n");
    printf("   -f <folder name>    Folder containing the images or sounds to pack.\n");
    printf("   -u <file name>      File to unpack from resource.\n");
    printf("   -c                  Compress the files.\n");
    printf("   -l                  Lists all files inside the resource.\n\n");
    printf("Example:\n");
    printf("   createres -r resources.dat -u image1.bmp\n");
    printf("   createres -r resources.dat -f DATA\n\n");

    exit(0);
}
