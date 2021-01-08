#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "command.h"

/*for the ls command*/
void listDir() {
	DIR *dir;
	struct dirent *sd;
	dir = opendir(".");
	while ((sd=readdir(dir)) != NULL) {
		if (strcmp(sd->d_name, ".") 
		&& strcmp(sd->d_name, "..")
		&& strcmp(sd->d_name, ".DS_Store")) {
			write(1, sd->d_name, strlen(sd->d_name));
			write(1, "\n", 1);
		}
	}
	closedir(dir);
} 

/*for the pwd command*/
void showCurrentDir() {
	char* currDir = NULL;
	char *buffer = (char *)malloc(256 * sizeof(char));
	currDir = getcwd(buffer, 256);
	if (currDir == NULL) {
		perror("Error! Cannot show current directory");
		return;
	}
	write(1, currDir, strlen(currDir));
	write(1, "\n", 1);
	free(buffer);
}

/*for the mkdir command*/
void makeDir(char *dirName) {
	DIR *dir;
	int stat;
	dir = opendir(dirName);
	if (dir) {
		perror("Directory already exist");
		closedir(dir);
		return;
	}
	else {
		// give all permissions
		stat = mkdir(dirName, 0777);
		if (stat == -1) {
			perror("mkdir failure");
		}
	}
} 

/*for the cd command*/
void changeDir(char *dirName) {
	// If we write no path (only 'cd'), then go to the home directory
	if (dirName == NULL) {
		chdir(getenv("HOME")); 
	}
	// Else we change the directory to the one specified by the 
	// argument, if possible
	else { 
		if (chdir(dirName) == -1) {
			perror("Error: no such directory exist");
			return;
		}
	}
}

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

/*for the cp command*/
void copyFile(char *sourcePath, char *destinationPath)
{
    int src;
    int dst;
    int nread;
    char buf[1];
    DIR *dst_dir;
    char *filename;
    char *destinationfile = (char *)malloc(32 * sizeof(char));
    strcpy(destinationfile, destinationPath);
    
    src = open(sourcePath, O_RDONLY);
    if (src < 0) {
        perror("Source file open failure");
        return;
    }

    dst_dir = opendir(destinationPath);
    if (dst_dir) {
        if (strstr(sourcePath, "/") != NULL) {
            filename = strrchr(sourcePath, '/');
            strcat(destinationfile, filename);
        }
        else {
            filename = sourcePath;
            if (strstr(destinationPath, "/") == NULL)
                strcat(destinationfile, "/");
            strcat(destinationfile, filename);
        }
        closedir(dst_dir);
    }
    dst = open(destinationfile, O_WRONLY | O_CREAT, 0700);

    if (dst < 0) {
        perror("Destination file create failure");
        return;
    }
	
    while((nread = read(src, buf, 1)) > 0) {
        write(dst, buf, 1);
    }

    free(destinationfile);
    close(src);
    close(dst);    
}

/*for the mv command*/
void moveFile(char *sourcePath, char *destinationPath) {
	copyFile(sourcePath, destinationPath);
	deleteFile(sourcePath);
} 

/*for the rm command*/
void deleteFile(char *filename) {
	int src;

	src = open(filename, O_RDONLY);
	if (src == -1) {
		perror("Error");
		return;
	} 
	else {
		close(src);
		remove(filename);
	}
} 


/*for the cat command*/
void displayFile(char *filename) {
	int src;
	int nread;
	char buf[1];
	src = open(filename, O_RDONLY);
	if (src == -1) {
		perror("File open failure");
		return;
	}
	while ((nread = read(src, buf, 1)) > 0) {
		write(1, buf, 1);
	}
	write(1, "\n", 1);
	close(src);
} 