#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include "command.h"

/*for the ls command*/
void listDir() {

} 

/*for the pwd command*/
void showCurrentDir() {

}

void makeDir(char *dirName) {

} /*for the mkdir command*/


/*for the cd command*/
void changeDir(char *dirName) {
	// If we write no path (only 'cd'), then go to the home directory
	if (dirName == NULL) {
		chdir(getenv("HOME")); 
	}
	// Else we change the directory to the one specified by the 
	// argument, if possible
	else{ 
		if (chdir(dirName == -1)) {
			printf(" %s: no such directory\n", dirName);
		}
	}
}

void copyFile(char *sourcePath, char *destinationPath) {

} /*for the cp command*/

void moveFile(char *sourcePath, char *destinationPath) {

} /*for the mv command*/

void deleteFile(char *filename) {

} /*for the rm command*/

void displayFile(char *filename) {

} /*for the cat command*/