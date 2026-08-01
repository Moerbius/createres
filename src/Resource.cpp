
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
    if (getcwd(pathname, sizeof(pathname)) == NULL) {
        perror("getcwd failure");
        exit(1);
    }
    
    //How many files are there?
    filecount = countfiles(path);
    printf("NUMBER OF FILES: %i\n", filecount);
    
    //Go back to the original path
    if (chdir(pathname) != 0) {
        perror("chdir failure");
        exit(1);
    }
    
    //Use the filename specified by the user
    fd = open(filename, O_WRONLY | O_EXCL | O_CREAT, S_IRUSR);
    
    //Did we get a valid file descriptor?
    if (fd < 0) {
        if (errno == EEXIST) {
            if (remove(filename) != 0) {
                printf("Unable to delete the file\n");
                perror("Error");
                exit(1);
            }

            fd = open(filename, O_WRONLY | O_EXCL | O_CREAT, S_IRUSR);
            if (fd < 0) {
                perror("Error creating the file");
                exit(1);
            }
        }
        else {
            perror("Error creating the file");
            exit(1);
        }
    }
    
    //Write the compression flag
    if (write(fd, &compress, sizeof(int)) != (ssize_t)sizeof(int)) {
        perror("Error writing compression flag");
        close(fd);
        exit(1);
    }
    
    //Write the total number of files as the first integer
    if (write(fd, &filecount, sizeof(int)) != (ssize_t)sizeof(int)) {
        perror("Error writing file count");
        close(fd);
        exit(1);
    }
    
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
    if (read(fd, &compress, sizeof(int)) != (ssize_t)sizeof(int)) {
        perror("Error reading compression flag");
        close(fd);
        exit(1);
    }
    
    //Read the first INT, which will tell us how many files are in this resource
    int numfiles;
    if (read(fd, &numfiles, sizeof(int)) != (ssize_t)sizeof(int) || numfiles < 0) {
        printf("Invalid resource file: %s\n", resourcefilename);
        close(fd);
        exit(1);
    }
    
    //Get the pointers to the stored files
    int *filestart = NULL;
    if (numfiles > 0) {
        filestart = (int *) malloc(sizeof(int) * numfiles);
        if (filestart == NULL) {
            perror("malloc failure");
            close(fd);
            exit(1);
        }
        if (read(fd, filestart, sizeof(int) * numfiles) != (ssize_t)(sizeof(int) * numfiles)) {
            printf("Invalid resource file index table: %s\n", resourcefilename);
            free(filestart);
            close(fd);
            exit(1);
        }
    }
    
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
        if (read(fd, filesize, sizeof(int)) != (ssize_t)sizeof(int) || *filesize < 0) {
            printf("Invalid resource file entry: %s\n", resourcefilename);
            free(filestart);
            close(fd);
            exit(1);
        }
        //Get the size of the filename string
        if (read(fd, &filenamesize, sizeof(int)) != (ssize_t)sizeof(int) || filenamesize < 0) {
            printf("Invalid resource file entry: %s\n", resourcefilename);
            free(filestart);
            close(fd);
            exit(1);
        }
        //Size the buffer and read the filename
        filename = (char *) malloc(filenamesize + 1);
        if (filename == NULL) {
            perror("malloc failure");
            free(filestart);
            close(fd);
            exit(1);
        }
        if (read(fd, filename, filenamesize) != filenamesize) {
            printf("Invalid resource file entry name: %s\n", resourcefilename);
            free(filename);
            free(filestart);
            close(fd);
            exit(1);
        }
        //Remember to terminate the string properly!
        filename[filenamesize] = '\0';
        //Compare to the string we're looking for
        if (strcmp(filename, resourcename) == 0)
        {
            //Read the compressed size
            int compressed_size;
            if (read(fd, &compressed_size, sizeof(int)) != (ssize_t)sizeof(int) || compressed_size < 0) {
                printf("Invalid resource file entry size: %s\n", resourcefilename);
                free(filename);
                free(filestart);
                close(fd);
                exit(1);
            }
            
            if(compress == 0) {
                if (compressed_size != *filesize) {
                    printf("Corrupt uncompressed entry size for '%s'\n", resourcename);
                    free(filename);
                    free(filestart);
                    close(fd);
                    exit(1);
                }
                //Read uncompressed contents
                buffer = (char *) malloc(*filesize > 0 ? *filesize : 1);
                if (buffer == NULL) {
                    perror("malloc failure");
                    free(filename);
                    free(filestart);
                    close(fd);
                    exit(1);
                }
                if (*filesize > 0 && read(fd, buffer, compressed_size) != compressed_size) {
                    printf("Error reading uncompressed entry '%s'\n", resourcename);
                    free(buffer);
                    free(filename);
                    free(filestart);
                    close(fd);
                    exit(1);
                }
            }
            else {
                //Read compressed contents and decompress
                char *compressed_buffer = (char *) malloc(compressed_size > 0 ? compressed_size : 1);
                if (compressed_buffer == NULL) {
                    perror("malloc failure");
                    free(filename);
                    free(filestart);
                    close(fd);
                    exit(1);
                }
                if (compressed_size > 0 &&
                    read(fd, compressed_buffer, compressed_size) != compressed_size) {
                    printf("Error reading compressed entry '%s'\n", resourcename);
                    free(compressed_buffer);
                    free(filename);
                    free(filestart);
                    close(fd);
                    exit(1);
                }
                
                std::string uncompressed;
                if (!snappy::Uncompress(compressed_buffer, compressed_size, &uncompressed) ||
                    (int)uncompressed.size() != *filesize) {
                    printf("Error decompressing entry '%s'\n", resourcename);
                    free(compressed_buffer);
                    free(filename);
                    free(filestart);
                    close(fd);
                    exit(1);
                }
                
                buffer = (char *) malloc(*filesize > 0 ? *filesize : 1);
                if (buffer == NULL) {
                    perror("malloc failure");
                    free(compressed_buffer);
                    free(filename);
                    free(filestart);
                    close(fd);
                    exit(1);
                }
                if (*filesize > 0) {
                    memcpy(buffer, uncompressed.data(), *filesize);
                }
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

int Resource::listFiles(char *resourcename) {
    
    int compression = 0;    //indicates if the resource file uses compression
    int numfiles = 0;       //Number of files inside resource
    int position;
    vector<int> positions;  //Position vector of all files
    int size;
    vector<int> sizes;
    int stored_size;
    vector<int> stored_sizes;
    int strsize;
    vector<string> names;   //Names vector of all files
    
    //Open the file for reading
    ifstream file (resourcename, ios::in | ios::binary);
    if(file.is_open()) {
        
        //Compression flag
        file.seekg(0, ios::beg);
        file.read(reinterpret_cast<char*>(&compression), sizeof(int));
        if (!file) {
            cout << "Could not read resource header from " << resourcename << endl;
            return 1;
        }
        
        //Number of files
        file.read(reinterpret_cast<char*>(&numfiles), sizeof(int));
        if (!file || numfiles < 0) {
            cout << "Invalid resource file: " << resourcename << endl;
            return 1;
        }
        
        //Gets the start byte position of each file
        for (int i = 0; i < numfiles; i++) {
            file.read(reinterpret_cast<char*>(&position), sizeof(int));
            if (!file) {
                cout << "Invalid resource file index table: " << resourcename << endl;
                return 1;
            }
            positions.push_back(position);
        }
        
        //Get the size, filename size and filename
        for (size_t i = 0; i < positions.size(); i++) {
            file.seekg(positions[i]);
            file.read(reinterpret_cast<char*>(&size), sizeof(int));
            file.read(reinterpret_cast<char*>(&strsize), sizeof(int));
            if (!file || strsize < 0) {
                cout << "Invalid resource file entry: " << resourcename << endl;
                return 1;
            }

            string name;
            name.resize(strsize);
            if (strsize > 0) {
                file.read(&name[0], strsize);
            }
            if (!file) {
                cout << "Invalid resource file entry name: " << resourcename << endl;
                return 1;
            }
            
            // Skip the compressed_size field (needed for unpacking but not displayed)
            int compressed_size;
            file.read(reinterpret_cast<char*>(&compressed_size), sizeof(int));
            if (!file || compressed_size < 0) {
                cout << "Invalid resource file entry size: " << resourcename << endl;
                return 1;
            }
            stored_size = compressed_size;
            
            sizes.push_back(size);
            stored_sizes.push_back(stored_size);
            names.push_back(name);
        }
        
        file.close();
        
    }
    else {
        cout << "Could not open the file " << resourcename << endl;
        return 1;
    }
    
    cout << "Compression: " << (compression == 0 ? "No" : "Yes") << endl;
    cout << "     #files: " << numfiles << endl << endl;
    cout << "       filename       | orig size | stored size | ratio | position " << endl;
    cout << "----------------------|-----------|-------------|-------|----------" << endl;
    
    for (int i = 0; i < numfiles; i++) {
        int ratio = 0;
        if (sizes[i] > 0) {
            ratio = (stored_sizes[i] * 100) / sizes[i];
        }

        cout << setw(21) << names[i]
             << " | " << setw(9) << sizes[i]
             << " | " << setw(11) << stored_sizes[i]
             << " | " << setw(3) << ratio << "%"
             << " | " << positions[i] << endl;
    }

    return 0;
    
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
    if (chdir(path) != 0) {
        perror("chdir failure");
        closedir(dir);
        exit(1);
    }
    
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
                    if (chdir("..") != 0) {
                        perror("chdir failure");
                        closedir(dir);
                        exit(1);
                    }
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
    if (write(fd, &currentloc, sizeof(int)) != (ssize_t)sizeof(int)) {
        perror("Error writing file offset");
        exit(1);
    }
    
    //Seek to the location where we'll be storing this new file info
    lseek(fd, currentloc, SEEK_SET);
    
    //Write the size of the file
    int filesize = getfilesize(filename);
    if (write(fd, &filesize, sizeof(filesize)) != (ssize_t)sizeof(filesize)) {
        perror("Error writing file size");
        exit(1);
    }
    totalsize += sizeof(int);
    
    //Write the LENGTH of the NAME of the file
    int filenamelen = (int)strlen(filename);
    if (write(fd, &filenamelen, sizeof(int)) != (ssize_t)sizeof(int)) {
        perror("Error writing filename length");
        exit(1);
    }
    totalsize += sizeof(int);
    
    //Write the name of the file
    if (write(fd, filename, filenamelen) != filenamelen) {
        perror("Error writing filename");
        exit(1);
    }
    totalsize += filenamelen;
    
    

    // Read the file contents
    int fd_read = open(filename, O_RDONLY);
    if (fd_read < 0) {
        perror("Error opening input file");
        exit(1);
    }

    char *buffer = (char *) malloc(filesize > 0 ? filesize : 1);
    if (buffer == NULL) {
        perror("malloc failure");
        close(fd_read);
        exit(1);
    }
    if (filesize > 0 && read(fd_read, buffer, filesize) != filesize) {
        perror("Error reading input file");
        free(buffer);
        close(fd_read);
        exit(1);
    }
    close(fd_read);
    
    if(compress == 0) {
        // Write uncompressed file contents
        int compressed_size = filesize;
        if (write(fd, &compressed_size, sizeof(int)) != (ssize_t)sizeof(int) ||
            (filesize > 0 && write(fd, buffer, filesize) != filesize)) {
            perror("Error writing file contents");
            free(buffer);
            exit(1);
        }
        totalsize += sizeof(int) + filesize;
    }
    else {
        // Compress using Snappy
        std::string compressed;
        snappy::Compress(buffer, filesize, &compressed);
        
        int compressed_size = (int)compressed.size();
        if (write(fd, &compressed_size, sizeof(int)) != (ssize_t)sizeof(int) ||
            (compressed_size > 0 &&
             write(fd, compressed.data(), compressed_size) != compressed_size)) {
            perror("Error writing compressed contents");
            free(buffer);
            exit(1);
        }
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
    if (chdir(path) != 0) {
        perror("chdir failure");
        closedir(dir);
        exit(1);
    }
    
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
                    if (chdir("..") != 0) {
                        perror("chdir failure");
                        closedir(dir);
                        exit(1);
                    }
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


