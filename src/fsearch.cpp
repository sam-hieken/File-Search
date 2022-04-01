/*
 * fsearch.cpp
 *
 *  Created on: Dec 19, 2021
 *      Author: Sam
 */

#include "fsearch.h"

static void f_add(char*);
static int fbig(FILE*);


static vector<string> v;
static vector<pthread_t> v_threads;

//static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[]) {
	char path[R_BUFFER_SIZE];
	char search[R_BUFFER_SIZE] = "";

	int advanced = 0;

	// Null check
	if (!getcwd(path, R_BUFFER_SIZE)) return -1;

	for (int i = 1; i < argc; i++) {
		if (strcmp("-a", argv[i]) == 0) advanced = 1;
		else {
			strcat(search, argv[i]);
			if (i != argc - 1) strcat(search, " ");
		}
	}

	cout << "Searching..." << endl << endl << endl;
	long l1 = time(NULL);

	if (!ends_with(path, "/") && !ends_with(path, "\\"))
		strcat(path, "/");

	searchDir(path, search, advanced);

	cout << "\nFOUND: " << v.size() << " files" << endl;
	cout << "TIME ELAPSED (s): " << time(NULL) - l1 << endl << endl;

	cout << "Exiting..." << endl;

	v.clear();

	return 0;
}

#ifdef _DIRENT_HAVE_D_TYPE // IF _DIRENT_HAVE_D_TYPE

/**
 * The search dir function if d_type is defined by the implementation
 */
void searchDir(char* dir_name, char* search, short advanced) {
	DIR* dir { opendir(dir_name) };

	// Null check
	if (!dir) {
		return;
	}

	dirent* dnt;

	while ((dnt = readdir(dir))) {

		if (DOTREF) continue;

		//	Full file path
		char dest[strlen(dnt->d_name) + strlen(dir_name) + 5];

		strcpy(dest, dir_name);
		strcat(dest, dnt->d_name);

		//	if directory entry name contains searched term...
		if (strstr(to_lower(dnt->d_name), to_lower(search))) {
			f_add(dest);
		}

		//	is it a directory? If so call search recursively on it
		if (dnt->d_type == DT_DIR) {

			strcat(dest, "/");
			searchDir(dest, search, advanced);
			continue;

		}

		//	If advanced search is turned on
		else if (advanced && dnt->d_type == DT_REG) {
			FILE* file = fopen(dest, "r");

			if(file == NULL || fbig(file)) {
				continue;
			}

			char ln[R_BUFFER_SIZE];

			while (fgets(ln, R_BUFFER_SIZE, file) != NULL) {
				if (strstr(to_lower(ln), to_lower(search))) {
					f_add(dest);
					break;
				}

			}

			fclose(file);
		}
	}

	closedir(dir);

	return;
}

#else

void* new_thread(void*);
static int is_dir(char*);

/**
 * The searchDir function if the implementation doesn't define d_type
 */
void searchDir(char* dir_name, char* search, short advanced, short firstRun) {
	DIR* dir { opendir(dir_name) };
	dirent* dnt;

	if (dir == NULL) return;

	while ((dnt = readdir(dir))) {
		if (DOTREF) continue;

		char dest[strlen(dnt->d_name) + strlen(dir_name) + 5];

		strcpy(dest, dir_name);
		strcat(dest, dnt->d_name);

		int isDir = is_dir(dest);

		//	if directory entry name contains searched term...
		if (strstr(to_lower(dnt->d_name), to_lower(search))) {
			f_add(dest);
		}

		//	is it a directory? if so, call this function recursively on it
		if (isDir) {
			if (strcmp(dnt->d_name, "WinSxS") == 0)
				continue;

			strcat(dest, "\\");

			//	first time through method? if so create new thread
			if (firstRun) {
				struct thread_args* ta = new thread_args;

				strcpy(ta->dir_name, dest);
				strcpy(ta->search, search);
				ta->advanced = advanced;

				pthread_t id;
				pthread_create(&id, NULL, &new_thread, ta);
				v_threads.push_back(id);
			}
			else
				searchDir(dest, search, advanced, 0);

			//	Why not continue? only thing left is check for advanced search if its a file (but its not)
			continue;
		}

		//	If advanced search is turned on
		else if (advanced && !isDir) {
			FILE* file = fopen(dest, "r");

			if(file == NULL || fbig(file)) {
				continue;
			}

			char ln[R_BUFFER_SIZE];

			while (fgets(ln, R_BUFFER_SIZE, file) != NULL) {

				if (strstr(to_lower(ln), to_lower(search))) {
					f_add(dest);
					break;
				}

			}

			fclose(file);
		}
	}

	//	If this is the first time
	if (firstRun) {

		for (pthread_t thread : v_threads) {

			pthread_join(thread, NULL);

		}

	}

	closedir(dir);

	return;
}

int count = 0;

void* new_thread(void* args) {
	struct thread_args* targs = (struct thread_args*) args;

	searchDir(targs->dir_name, targs->search, targs->advanced, 0);

	delete targs;

	return NULL;
}

static int is_dir(char* file) {

#ifdef _WIN32 // IF WIN32
	WIN32_FIND_DATA fileData;
	HANDLE			hSearch;

	hSearch = FindFirstFile(TEXT(file), &fileData);

	if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		FindClose(hSearch);
		return 1;
	}

	FindClose(hSearch);
	return 0;

#else
	struct stat info;
	stat(dest, &info);

	return S_ISDIR(info.st_mode);

#endif // ENDIF WIN32
}

#endif // ENDIF _DIRENT_HAVE_D_TYPE



/**
 * Returns: 1 if file longer than max file size (5MB?), 0 if not
 */
static int fbig(FILE* file) {
	fseek(file, 0L, SEEK_END);
	long sz = ftell(file);
	rewind(file);

	if(sz > MAX_FILE_LENGTH || sz == -1)
		return 1;

	else return 0;
}




/**
 * Adds file path to vector
 */
static void f_add(char* dest) {
	string to(dest);

	v.push_back(to);
	cout << to << endl;
}




char* to_lower(char* str) {
	for (unsigned i = 0; i < strlen(str); i++){
		if(str[i] >= 65 && str[i] <= 90) str[i] = str[i] + 32;
	}

	return str;
}



/**
 * From StackOverflow, modified for speed
 */
int ends_with(const char *str, const char *suffix)
{
	if (!str || !suffix)
		return 0;
	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	if (lensuffix >  lenstr)
		return 0;
	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}
