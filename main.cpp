
#ifdef _MSC_VER
	#include "direntVC.h"
	#include "getopt.h"
	#include <io.h>
#include <direct.h>
	#define MAXPATHLEN	MAX_PATH
	#define MYCHAR		wchar_t
#else
	#include <dirent.h>
	#include <sys/param.h>
	#include <unistd.h>
	#define MYCHAR		char
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <ctype.h>

#define VERSION "2.0"

/*
BYTELOC      DATA        EXPLANATION
*******      ****        ***********
TODO: 0            1           (0 -> No compression 1 -> Compression)
1-4          3           (Integer indicating that 3 files are stored in this resource)
5-8          16          (Integer indicating that the first file is stored from the 16th byte onward)
9-12         40          (Integer indicating that the second file is stored from the 40th byte onward)
13-16        10056       (Integer indicating that the third file is stored from the 10056th byte onward)
17-20        9           (Integer indicating that the first stored file contains 9 bytes of data)
21-24        8           (Integer indicating that the first stored file's name is 8 characters in length)
25-32        TEST.TXT    (7 bytes, each encoding one character of the first stored file's filename)
33-41        Testing12   (9 bytes, containing the first stored file's data, which happens to be some text)
42-45        10000       (Integer indicating that the second stored file contains 10000 bytes of data)
46-49        9           (Integer indicating that the second stored file's name is 9 characters in length)
50-58        TEST2.BMP   (8 bytes, each encoding one character of the second stored file's filename)
59-10058     ...         (10000 bytes, representing the data stored within TEST2.BMP.  Data not shown!)
10059-10062  20000       (Integer indicating that the third stored file contains 20000 bytes of data)
10063-10066  9           (Integer indicating that the third stored file's name is 9 characters in length)
10067-10075  TEST3.WAV   (8 bytes, each encoding one character of the third stored file's filename)
10076-30075  ...         (20000 bytes, representing the data stored within TEST3.WAV.  Data not shown!)
*/

//Function prototypes
void pack(char *filename, char *path);
int getfilesize(char *filename);
int countfiles(char *path);
void packfile(char *filename, int fd);
void findfiles(char *path, int fd);
void showHelp();
void showFiles(char *filename);

int currentfile = 1;	//This integer indicates what file we're currently adding to the resource.
int currentloc = 0;	    //This integer references the current write-location within the resource file

int main(int argc, MYCHAR *argv[]) {
	
	char *arguments = "hf:p:";

// Need to convert char* to wchar_t*
#ifdef _MSC_VER
	wchar_t *argumentlist;
	mbtowc(argumentlist, arguments, sizeof(arguments));
#else
	char *argumentlist = arguments;
#endif

    MYCHAR *filename;
    MYCHAR *path;
	
    int c;

    if(argc == 1)
        showHelp();

	while ((c = getopt(argc, argv, argumentlist)) != -1)
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
    }

    if(filename != NULL && path != NULL)
        pack((char *)filename, (char*)path);

    for (int index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);

    return 0;
}

void pack(char *filename, char *path)
{
    char pathname[MAXPATHLEN+1];	//This character array will hold the app's working directory path
	int filecount;		            //How many files are we adding to the resource?
	int fd;				            //The file descriptor for the new resource

	//Store the current path
	_getcwd(pathname, sizeof(pathname));

	//How many files are there?
	filecount = countfiles(path);
	printf("NUMBER OF FILES: %i\n", filecount);

	//Go back to the original path
	_chdir(pathname);

    //Use the filename specified by the user
    fd = _open(filename, O_WRONLY | O_EXCL | O_CREAT | O_BINARY, S_IRUSR);

	//Did we get a valid file descriptor?
	if (fd < 0)
	{
		//Can't create the file for some reason (possibly because the file already exists)
		perror("Cannot create resource file");
		exit(1);
	}

	//Write the total number of files as the first integer
	_write(fd, &filecount, sizeof(int));

	//Set the current conditions
	currentfile = 1;					//Start off by storing the first file, obviously!
	currentloc = (sizeof(int) * filecount) + sizeof(int);	//Leave space at the begining for the header info

	//Use the findfiles routine to pack in all the files
	findfiles(path, fd);

	//Close the file
	_close(fd);
}

int getfilesize(char *filename) {

	struct stat file;	//This structure will be used to query file status

	//Extract the file status info
	if(!stat(filename, &file))
	{
		//Return the file size
		return file.st_size;
	}

	//ERROR! Couldn't get the filesize.
	printf("getfilesize:  Couldn't get filesize of '%s'.", filename);
	exit(1);
}

int countfiles(char *path) {

	int count = 0;			//This integer will count up all the files we encounter
	struct dirent *entry;		//This structure will hold file information
	struct stat file_status;	//This structure will be used to query file status
	DIR *dir = opendir(path);	//This pointer references the directory stream

	//Make sure we have a directory stream pointer
	if (!dir) {
		perror("opendir failure");
		exit(1);
	}

	//Change directory to the given path
	_chdir(path);

	//Loop through all files and directories
	while ( (entry = readdir(dir)) != NULL) {
		//Don't bother with the .. and . directories
		if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0)) {
			//Get the status info for the current file
			if (stat(entry->d_name, &file_status) == 0) {
				//Is this a directory, or a file?
				if (S_ISDIR(file_status.st_mode)) {
					//Call countfiles again (recursion) and add the result to the count total
					count += countfiles(entry->d_name);
					_chdir("..");
				}
				else {
					//We've found a file, increment the count
					count++;
				}
			}
		}
	}

	//Make sure we close the directory stream
	if (closedir(dir) == -1) {
		perror("closedir failure");
		exit(1);
	}

	//Return the file count
	return count;
}

void packfile(char *filename, int fd) {

	int totalsize = 0;	//This integer will be used to track the total number of bytes written to file

	//Handy little output
	printf("PACKING: '%s' SIZE: %i\n", filename, getfilesize(filename));

	//In the 'header' area of the resource, write the location of the file about to be added
	_lseek(fd, currentfile * sizeof(int), SEEK_SET);
	_write(fd, &currentloc, sizeof(int));

	//Seek to the location where we'll be storing this new file info
	_lseek(fd, currentloc, SEEK_SET);

	//Write the size of the file
	int filesize = getfilesize(filename);
	_write(fd, &filesize, sizeof(filesize));
	totalsize += sizeof(int);

	//Write the LENGTH of the NAME of the file
	int filenamelen = strlen(filename);
	_write(fd, &filenamelen, sizeof(int));
	totalsize += sizeof(int);

	//Write the name of the file
	_write(fd, filename, strlen(filename));
	totalsize += strlen(filename);

	//Write the file contents
	int fd_read = open(filename, O_RDONLY);		//Open the file
	char *buffer = (char *) malloc(filesize);	//Create a buffer for its contents
	_read(fd_read, buffer, filesize);		//Read the contents into the buffer
	_write(fd, buffer, filesize);			//Write the buffer to the resource file
	_close(fd_read);					//Close the file
	free(buffer);					//Free the buffer
	totalsize += filesize;				//Add the file size to the total number of bytes written

	//Increment the currentloc and current file values
	currentfile++;
	currentloc += totalsize;
}

void findfiles(char *path, int fd) {

	struct dirent *entry;		//This structure will hold file information
	struct stat file_status;	//This structure will be used to query file status
	DIR *dir = opendir(path);	//This pointer references the directory stream

	//Make sure we have a directory stream pointer
	if (!dir) {
		perror("opendir failure");
		exit(1);
	}

	//Change directory to the given path
	_chdir(path);

	//Loop through all files and directories
	while ( (entry = readdir(dir)) != NULL) {
		//Don't bother with the .. and . directories
		if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0)) {
			//Get the status info for the current file
			if (stat(entry->d_name, &file_status) == 0) {
				//Is this a directory, or a file?
				if (S_ISDIR(file_status.st_mode)) {
					//Call findfiles again (recursion), passing the new directory's path
					findfiles(entry->d_name, fd);
					_chdir("..");
				}
				else {
					//We've found a file, pack it into the resource file
					packfile(entry->d_name, fd);
				}
			}
		}
	}

	//Make sure we close the directory stream
	if (closedir(dir) == -1) {
		perror("closedir failure");
		exit(1);
	}

	return;
}

void showHelp()
{
    //customresource resource myresource.dat
    printf("Resource Packer %s\n\n", VERSION);
    printf("Usage:\n");
    printf("createres <option> <option>\n\n");
    printf("   -f   Output file name\n");
    printf("   -p   Folder containing the images or sounds to pack.\n");
    printf("   -s   Show the files inside the resource file. (NOT AVAILABLE YET)\n");
    printf("Example:\n");
    printf("   createres -f resources.dat -p DATA\n");
    exit(0);
}