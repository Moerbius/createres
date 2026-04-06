
#include "Resource.h"

Resource::Resource() {
    
    compresslloc = 0x00;
    numfilesloc = 0x04;
    firstfileloc = 0;
    
    currentfile = 1;	//This integer indicates what file we're currently adding to the resource.
    currentloc = 0;	    //This integer references the current write-location within the resource file
    compress = 0;       //Indicates either to use compression or not. 0 -> not compressed, 1 -> compressed
};

Resource::~Resource() {
    
};

void Resource::pack(char *filename, char *path) {
    char pathname[MAXPATHLEN+1];	//This character array will hold the app's working directory path
    int filecount;		            //How many files are we adding to the resource?
    int fd;				            //The file descriptor for the new resource
    
    //Store the current path
    getcwd(pathname, sizeof(pathname));
    
    //How many files are there?
    filecount = countfiles(path);
    printf("NUMBER OF FILES: %i\n", filecount);
    
    //Go back to the original path
    chdir(pathname);
    
    //Use the filename specified by the user
    fd = open(filename, O_WRONLY | O_EXCL | O_CREAT, S_IRUSR);
    
    //Did we get a valid file descriptor?
    if (fd < 0) {
        if (errno == EEXIST) {
            
            close(fd);
            
            int removestatus = remove(filename);
            
            if( removestatus == 0 ) {
                fd = open(filename, O_WRONLY | O_EXCL | O_CREAT, S_IRUSR);
                
                if(fd < 0) {
                    perror("Error creating the file");
                    exit(1);
                }
            }
            else
            {
                printf("Unable to delete the file");
                perror("Error");
            }
        }
        else {
            perror("Error creating the file");
            exit(1);
        }
    }
    
    //Write the compressin flag
    write(fd, &compress, sizeof(int));
    
    //Advance to next position
    //lseek(fd, sizeof(int)*2, SEEK_SET);
    
    //Write the total number of files as the first integer
    write(fd, &filecount, sizeof(int));
    
    //lseek(fd, sizeof(int), SEEK_SET);
    
    //Set the current conditions
    currentfile = 1;					//Start off by storing the first file, obviously!
    currentloc = (sizeof(int) * filecount) + (sizeof(int)*2);	//Leave space at the begining for the header info
    
    //Use the findfiles routine to pack in all the files
    findfiles(path, fd);
    
    //Close the file
    close(fd);
}

char *Resource::unpack(char *resourcefilename, char *resourcename, int *filesize) {
    
    //Try to open the resource file in question
    int fd = open(resourcefilename, O_RDONLY);
    if (fd < 0)
    {
        perror("Error opening resource file");
        exit(1);
    }
    
    //Make sure we're at the beginning of the file
    lseek(fd, 0, SEEK_SET);
    
    //Get the compress flag
    read(fd, &compress, sizeof(int));
    
    //Read the first INT, which will tell us how many files are in this resource
    int numfiles;
    read(fd, &numfiles, sizeof(int));
    
    //Get the pointers to the stored files
    int *filestart = (int *) malloc(sizeof(int) * numfiles);
    read(fd, filestart, sizeof(int) * numfiles);
    
    //Loop through the files, looking for the file in question
    int filenamesize;
    char *buffer = NULL;
    int i;
    for(i=0;i<numfiles;i++)
    {
        char *filename;
        //Seek to the location
        lseek(fd, filestart[i], SEEK_SET);
        //Get the filesize value (original uncompressed)
        read(fd, filesize, sizeof(int));
        //Get the size of the filename string
        read(fd, &filenamesize, sizeof(int));
        //Size the buffer and read the filename
        filename = (char *) malloc(filenamesize + 1);
        read(fd, filename, filenamesize);
        //Remember to terminate the string properly!
        filename[filenamesize] = '\0';
        //Compare to the string we're looking for
        if (strcmp(filename, resourcename) == 0)
        {
            //Read the compressed size
            int compressed_size;
            read(fd, &compressed_size, sizeof(int));
            
            if(compress == 0) {
                //Read uncompressed contents
                buffer = (char *) malloc(*filesize);
                read(fd, buffer, compressed_size);
            }
            else {
                //Read compressed contents and decompress
                char *compressed_buffer = (char *) malloc(compressed_size);
                read(fd, compressed_buffer, compressed_size);
                
                std::string uncompressed;
                snappy::Uncompress(compressed_buffer, compressed_size, &uncompressed);
                
                buffer = (char *) malloc(*filesize);
                memcpy(buffer, uncompressed.data(), *filesize);
                free(compressed_buffer);
            }
            free(filename);
            break;
        }
        //Free the filename buffer
        free(filename);
    }
    
    //Release memory
    free(filestart);
    
    //Close the resource file!
    close(fd);
    
    //Did we find the file within the resource that we were looking for?
    if (buffer == NULL)
    {
        printf("Unable to find '%s' in the resource file!\n", resourcename);
        exit(1);
    }
    
    //Return the buffer
    return buffer;
};

void Resource::listFiles(char *resourcename) {
    
    int compression;        //indicates if the resource file uses compression
    int numfiles;           //Number of files inside resource
    int position;
    vector<int> positions;  //Position vector of all files
    int size;
    vector<int> sizes;
    int strsize;
    vector<int> strsizes;   //Size vector of all files
    char *name;
    vector<char*> names;   //Names vector of all files
    
    //Open the file for reading
    ifstream file (resourcename, ios::in | ios::binary);
    if(file.is_open()) {
        
        //Compressin flag
        file.seekg(0, ios::beg);
        file.read(reinterpret_cast<char*>(&compression), sizeof(int));
        
        //Number of files
        file.read(reinterpret_cast<char*>(&numfiles), sizeof(int));
        
        //Gets the start byte positin of each file
        for (int i = 0; i < numfiles; i++) {
            file.read(reinterpret_cast<char*>(&position), sizeof(int));
            positions.push_back(position);
        }
        
        //Get the size, filename size and filename
        for (size_t i = 0; i < positions.size(); i++) {
            file.seekg(positions[i]);
            file.read(reinterpret_cast<char*>(&size), sizeof(int));
            file.read(reinterpret_cast<char*>(&strsize), sizeof(int));
            name = new char[strsize + 1];
            file.read((name), strsize);
            name[strsize] = '\0';
            
            // Skip the compressed_size field (needed for unpacking but not displayed)
            int compressed_size;
            file.read(reinterpret_cast<char*>(&compressed_size), sizeof(int));
            
            sizes.push_back(size);
            strsizes.push_back(strsize);
            names.push_back(name);
        }
        
        file.close();
        
    }
    else {
        cout << "Could not open the file " << resourcename << endl;
    }
    
    cout << "Compression: " << (compression == 0 ? "No" : "Yes") << endl;
    cout << "     #files: " << numfiles << endl << endl;;
    cout << "       filename       | file size | position " << endl;
    cout << "----------------------|-----------|----------" << endl;
    
    for (int i = 0; i < numfiles; i++) {
        cout << setw(21) << names[i] << " | " << setw(9) << sizes[i] << " | " << positions[i] << endl;
    }

    for (int i = 0; i < numfiles; i++) {
        delete[] names[i];
    }
    
}

int Resource::getfilesize(char *filename) {
    
    struct stat file;	//This structure will be used to query file status
    
    //Extract the file status info
    if(!stat(filename, &file))
    {
        //Return the file size
        return (int)file.st_size;
    }
    
    //ERROR! Couldn't get the filesize.
    printf("getfilesize:  Couldn't get filesize of '%s'.", filename);
    exit(1);
}

int Resource::countfiles(char *path) {
    
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
        if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0) && (strcmp(entry->d_name, ".DS_Store") != 0)) {
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

void Resource::packfile(char *filename, int fd) {
    
    int totalsize = 0;	//This integer will be used to track the total number of bytes written to file
    
    //Handy little output
    printf("PACKING: '%s' SIZE: %i\n", filename, getfilesize(filename));
    
    //In the 'header' area of the resource, write the location of the file about to be added
    lseek(fd, currentfile * sizeof(int) + sizeof(int), SEEK_SET);
    write(fd, &currentloc, sizeof(int));
    
    //Seek to the location where we'll be storing this new file info
    lseek(fd, currentloc, SEEK_SET);
    
    //Write the size of the file
    int filesize = getfilesize(filename);
    write(fd, &filesize, sizeof(filesize));
    totalsize += sizeof(int);
    
    //Write the LENGTH of the NAME of the file
    int filenamelen = (int)strlen(filename);
    write(fd, &filenamelen, sizeof(int));
    totalsize += sizeof(int);
    
    //Write the name of the file
    write(fd, filename, strlen(filename));
    totalsize += strlen(filename);
    
    

    // Read the file contents
    int fd_read = open(filename, O_RDONLY);
    char *buffer = (char *) malloc(filesize);
    read(fd_read, buffer, filesize);
    close(fd_read);
    
    if(compress == 0) {
        // Write uncompressed file contents
        int compressed_size = filesize;
        write(fd, &compressed_size, sizeof(int));
        write(fd, buffer, filesize);
        totalsize += sizeof(int) + filesize;
    }
    else {
        // Compress using Snappy
        std::string compressed;
        snappy::Compress(buffer, filesize, &compressed);
        
        int compressed_size = (int)compressed.size();
        write(fd, &compressed_size, sizeof(int));
        write(fd, compressed.data(), compressed_size);
        totalsize += sizeof(int) + compressed_size;
        
        printf("   Compressed from %d to %d bytes\n", filesize, compressed_size);
    }
    
    free(buffer);
    
    
    
    //Increment the currentloc and current file values
    currentfile++;
    currentloc += totalsize;
}

void Resource::findfiles(char *path, int fd) {
    
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
        if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0) && (strcmp(entry->d_name, ".DS_Store") != 0)) {
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






int Resource::chartoint(char *value) {
    
    //if(sizeof(value) < sizeof(int)) {
        return (value[3] << 24) | (value[2] << 16) | (value[1] << 8) | (value[0]);
    /*}
    else {
        printf("Value too big to convert to int\n");
        return -1;
    }*/
    
}


