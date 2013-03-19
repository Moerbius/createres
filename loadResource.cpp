
#ifdef _MSC_VER
	#include <io.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>

char *GetBufferFromResource(char *resourcefilename, char *resourcename, int *filesize)
{
	//Try to open the resource file in question
	int fd = _open(resourcefilename, O_RDONLY);
	if (fd < 0)
	{
		perror("Error opening resource file");
		exit(1);
	}

	//Make sure we're at the beginning of the file
	_lseek(fd, 0, SEEK_SET);

	//Read the first INT, which will tell us how many files are in this resource
	int numfiles;
	_read(fd, &numfiles, sizeof(int));

	//Get the pointers to the stored files
	int *filestart = (int *) malloc(numfiles);
	_read(fd, filestart, sizeof(int) * numfiles);

	//Loop through the files, looking for the file in question
	int filenamesize;
	char *buffer;
	int i;
	for(i=0;i<numfiles;i++)
	{
		char *filename;
		//Seek to the location
		_lseek(fd, filestart[i], SEEK_SET);
		//Get the filesize value
		_read(fd, filesize, sizeof(int));
		//Get the size of the filename string
		_read(fd, &filenamesize, sizeof(int));
		//Size the buffer and read the filename
		filename = (char *) malloc(filenamesize + 1);
		_read(fd, filename, filenamesize);
		//Remember to terminate the string properly!
		filename[filenamesize] = '\0';
		//Compare to the string we're looking for
		if (strcmp(filename, resourcename) == 0)
		{
			//Get the contents of the file
			buffer = (char *) malloc(*filesize);
			_read(fd, buffer, *filesize);
			free(filename);
			break;
		}
		//Free the filename buffer
		free(filename);
	}

	//Release memory
	free(filestart);

	//Close the resource file!
	_close(fd);

	//Did we find the file within the resource that we were looking for?
	if (buffer == NULL)
	{
		printf("Unable to find '%s' in the resource file!\n", resourcename);
		exit(1);
	}

	//Return the buffer
	return buffer;
}