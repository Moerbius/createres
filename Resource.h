
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

#ifndef __createres__Resource__
#define __createres__Resource__

#ifdef _MSC_VER
#include "VC/direntVC.h"
#include <io.h>
#include <direct.h>
#include <tchar.h>
#define MAXPATHLEN	MAX_PATH
#else
#include <dirent.h>
#include <sys/param.h>
#include <unistd.h>
#endif

#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

class Resource {
    
    public:
        Resource();
        ~Resource();
        void pack(char *filename, char *path);
        char *unpack(char *resourcefilename, char *resourcename, int *filesize);
        int compress;       //Indicates either to use compression or not
        void listFiles(char *resourcename);
    
    private:
        int getfilesize(char *filename);
        int countfiles(char *path);
        void packfile(char *filename, int fd);
        void findfiles(char *path, int fd);
        void showFiles(char *filename);
        int chartoint(char *value);
    
        int currentfile;	//This integer indicates what file we're currently adding to the resource.
        int currentloc;	    //This integer references the current write-location within the resource file
    
};

#endif /* defined(__createres__Resource__) */
