/*
 * fsearch.h
 *
 *  Created on: Dec 19, 2021
 *      Author: Sam
 */

#ifndef FSEARCH_H_
#define FSEARCH_H_

#include <iostream>
#include <vector>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif

// If the directory is the . (current) or .. (previous) directory
#define DOTREF strcmp(dnt->d_name, ".") == 0 || strcmp(dnt->d_name, "..") == 0

#define R_BUFFER_SIZE 1024
#define MAX_FILE_LENGTH 5000000L

using namespace std;
typedef struct dirent dirent;

struct thread_args {
	char dir_name[256];
	char search[256];

	short advanced;
};

#ifdef _DIRENT_HAVE_D_TYPE
void searchDir(char*, char*, short);

#else
void searchDir(char*, char*, short);

#endif



char* to_lower(char*);
int ends_with(const char*, const char*);


#endif /* FSEARCH_H_ */
