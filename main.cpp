#include "stdio.h"
#include "dirent.h"
#include "sys/stat.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/param.h"
#include <cstring>
#include <cstdlib>

/*
BYTELOC      DATA        EXPLANATION
*******      ****        ***********
0-3          3           (Integer indicating that 3 files are stored in this resource)
4-7          16          (Integer indicating that the first file is stored from the 16th byte onward)
8-11         40          (Integer indicating that the second file is stored from the 40th byte onward)
12-15        10056       (Integer indicating that the third file is stored from the 10056th byte onward)
16-19        9           (Integer indicating that the first stored file contains 9 bytes of data)
20-23        8           (Integer indicating that the first stored file's name is 8 characters in length)
24-31        TEST.TXT    (7 bytes, each encoding one character of the first stored file's filename)
32-40        Testing12   (9 bytes, containing the first stored file's data, which happens to be some text)
41-44        10000       (Integer indicating that the second stored file contains 10000 bytes of data)
45-48        9           (Integer indicating that the second stored file's name is 9 characters in length)
49-57        TEST2.BMP   (8 bytes, each encoding one character of the second stored file's filename)
58-10057     ...         (10000 bytes, representing the data stored within TEST2.BMP.  Data not shown!)
10058-10061  20000       (Integer indicating that the third stored file contains 20000 bytes of data)
10062-10065  9           (Integer indicating that the third stored file's name is 9 characters in length)
10066-10074  TEST3.WAV   (8 bytes, each encoding one character of the third stored file's filename)
10075-30074  ...         (20000 bytes, representing the data stored within TEST3.WAV.  Data not shown!)
*/

//Function prototypes
int getfilesize(char *filename);
int countfiles(char *path);
void packfile(char *filename, int fd);
void findfiles(char *path, int fd);
void showHelp();
void showFiles(char *filename);

int currentfile = 1;  //This integer indicates what file we're currently adding to the resource.
int currentloc = 0;	    //This integer references the current write-location within the resource file

int main(int argc, char *argv[]) {

    int optind = 1;

    if(argc == 1)
    {
        showHelp();
        exit(0);
    }
    
    if(argc == 2 && strcmp(argv[2], "-h"))
    {
        showHelp();
        exit(0);
    }

    while ((optind < argc) && (argv[optind][0]=='-'))
    {
        char *sw = argv[optind];
        char *filename;
        char *folder;
        
        if(strcmp(sw, "-f") == 0)
        {
            optind++;
            //showHelp();
            printf("F");
        }
        else if(strcmp(sw, "-n") == 0)
        {
            optind++;
            printf("N");
        }
        else
        {
            optind++;
            showHelp();
            exit(0);
        }

    }




/*
	char pathname[MAXPATHLEN+1];	//This character array will hold the app's working directory path
	int filecount;		            //How many files are we adding to the resource?
	int fd;				            //The file descriptor for the new resource

	//Store the current path
	getcwd(pathname, sizeof(pathname));

	//How many files are there?
	filecount = countfiles(argv[1]);
	printf("NUMBER OF FILES: %i\n", filecount);

	//Go back to the original path
	chdir(pathname);

	//How many arguments did the user pass?
	if (argc < 3)
	{
		//The user didn't specify a resource file name, go with the default
		fd = open("resource.dat", O_WRONLY | O_EXCL | O_CREAT | O_BINARY, S_IRUSR);
	}
	else
	{
		//Use the filename specified by the user
		fd = open(argv[2], O_WRONLY | O_EXCL | O_CREAT | O_BINARY, S_IRUSR);
	}
	//Did we get a valid file descriptor?
	if (fd < 0)
	{
		//Can't create the file for some reason (possibly because the file already exists)
		perror("Cannot create resource file");
		exit(1);
	}

	//Write the total number of files as the first integer
	write(fd, &filecount, sizeof(int));

	//Set the current conditions
	currentfile = 1;					//Start off by storing the first file, obviously!
	currentloc = (sizeof(int) * filecount) + sizeof(int);	//Leave space at the begining for the header info

	//Use the findfiles routine to pack in all the files
	findfiles(argv[1], fd);

	//Close the file
	close(fd);
*/
	return 0;
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
	chdir(path);

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
					chdir("..");
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
	lseek(fd, currentfile * sizeof(int), SEEK_SET);
	write(fd, &currentloc, sizeof(int));

	//Seek to the location where we'll be storing this new file info
	lseek(fd, currentloc, SEEK_SET);

	//Write the size of the file
	int filesize = getfilesize(filename);
	write(fd, &filesize, sizeof(filesize));
	totalsize += sizeof(int);

	//Write the LENGTH of the NAME of the file
	int filenamelen = strlen(filename);
	write(fd, &filenamelen, sizeof(int));
	totalsize += sizeof(int);

	//Write the name of the file
	write(fd, filename, strlen(filename));
	totalsize += strlen(filename);

	//Write the file contents
	int fd_read = open(filename, O_RDONLY);		//Open the file
	char *buffer = (char *) malloc(filesize);	//Create a buffer for its contents
	read(fd_read, buffer, filesize);		//Read the contents into the buffer
	write(fd, buffer, filesize);			//Write the buffer to the resource file
	close(fd_read);					//Close the file
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
	chdir(path);

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
					chdir("..");
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
    printf("Resource Packer\n\n");
    printf("Usage:\n");
    printf("createres <folder> <filename>\n\n");
    printf("<folder>    Frolder containing the images or sounds to pack.\n");
    printf("<filename> Resource file name (optional)\n");
}
