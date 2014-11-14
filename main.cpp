#ifdef _MSC_VER
#include "VC/direntVC.h"
#include "VC/getopt.h"
#include <io.h>
#include <direct.h>
#include <tchar.h>
#define MAXPATHLEN	MAX_PATH
#else
#include <dirent.h>
#include <sys/param.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <ctype.h>

#include "Resource.h"

#define VERSION "2.1.0"
#define ARGUMENTS "hf:p:"

void showHelp();

#ifdef _MSC_VER
int _tmain(int argc, TCHAR** argv) {
    //Disable -> warning C4996: 'abc': The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name: _abc. See online help for details.
#pragma warning (disable : 4996)
    
    wchar_t *arguments = _T(ARGUMENTS);
    wchar_t *filename;
    wchar_t *path;
    
#else
    int main(int argc, char** argv) {
        const char *arguments = ARGUMENTS;
        char *filename = NULL;
        char *path = NULL;
#endif
        // Our Resource Class
        Resource resources;
        
        int c;
        
        if(argc == 1)
            showHelp();
        
        c = getopt(argc, argv, arguments);
        
        while (c != -1)
        {
            switch (c)
            {
                case 'h':
                    showHelp();
                    break;
                case 'f':
                    filename = optarg;
                    break;
                case 'p':
                    path = optarg;
                    break;
                case '?':
                    if (optopt == ':')
                        fprintf (stderr, "Option -%c requires an argument.\n", optopt);
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
        
#ifdef _MSC_VER
        
        char newfilename[255];
        char newpath[255];
        
        if(filename != NULL && path != NULL)
        {
            
            int i = 0;
            
            while(filename[i] != '\0')
            {
                newfilename[i] = (char)filename[i];
                ++i;
            }
            newfilename[i] = '\0';
            i = 0;
            
            while(path[i] != '\0')
            {
                newpath[i] = (char)path[i];
                ++i;
            }
            newpath[i] = '\0';
            
            resources.pack(newfilename, newpath);
#else
            if(filename != NULL && path != NULL)
            {
                resources.pack(filename, path);
#endif
                
            }
            
            for (int index = optind; index < argc; index++)
                printf ("Non-option argument %s\n", argv[index]);
            
            return 0;
        }
        
        void showHelp() {
            //customresource resource myresource.dat
            printf("Resource Packer %s\n\n", VERSION);
            printf("Usage:\n");
            printf("createres <option> <option>\n\n");
            printf("   -f   Output file name\n");
            printf("   -p   Folder containing the images or sounds to pack.\n");
            printf("   -c   Compress the files. (NOT AVAILABLE YET)\n");
            printf("   -s   Show the files inside the resource file. (NOT AVAILABLE YET)\n");
            printf("Example:\n");
            printf("   createres -f resources.dat -p DATA\n");
            
            exit(0);
        }
