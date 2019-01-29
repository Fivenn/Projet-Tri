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
	if(pid == -1){ //Création du fork échoue
		printf("fork failed\n");
		exit(EXIT_FAILURE);
	}else if(pid == 0){ //Partie fils
		char buffer[SU_MAXLINESIZE];
		snprintf(buffer, SU_MAXLINESIZE, "rm %s",file);
		execl("/bin/sh", "sh", "-c", path, (char *) NULL); //recouvrement du code avec la commande execl
	} else { //Partie père
		waitpid(pid, &status, 0); //En attente du PID du fils pour continuer son exe
	}
}
