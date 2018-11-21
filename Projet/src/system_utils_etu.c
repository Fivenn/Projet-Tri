/**
 * @file 
 * @brief Implementation by students of usefull function for the system project.
 * @todo Change the SU_removeFile to use exec instead of system.
 */


#include "system_utils.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
 * @brief Maximum length (in character) for a command line.
 **/
#define SU_MAXLINESIZE (1024*8) 

/********************** File managment ************/

void SU_removeFile(const char * file){
	int status;
	pid_t pid=fork();
	if(-1==pid){
		printf("fork() failed\n");
		exit(EXIT_FAILURE);
	}else if(pid==0){
		char path[50];
		strcpy(path, "rm ");
		strcat(path, file);
		printf("%s\n", path);
		execl("/bin/sh", "sh", "-c", path, (char *) NULL);
	}else{
		printf("[%d]fork with id %d\n", pid, pid);
		waitpid(pid, &status, 0);
	}
}